/*
 * This file is part of DisOrder.
 * Copyright (C) 2004-2008 Richard Kettlewell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include "types.h"

#include <stdio.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#include <locale.h>
#include <syslog.h>
#include <sys/time.h>
#include <pcre.h>
#include <fcntl.h>
#include <gcrypt.h>

#include "daemonize.h"
#include "event.h"
#include "log.h"
#include "configuration.h"
#include "rights.h"
#include "trackdb.h"
#include "queue.h"
#include "mem.h"
#include "play.h"
#include "server.h"
#include "state.h"
#include "syscalls.h"
#include "defs.h"
#include "user.h"
#include "mixer.h"
#include "eventlog.h"
#include "printf.h"
#include "version.h"

static ev_source *ev;

static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'V' },
  { "config", required_argument, 0, 'c' },
  { "debug", no_argument, 0, 'd' },
  { "foreground", no_argument, 0, 'f' },
  { "log", required_argument, 0, 'l' },
  { "pidfile", required_argument, 0, 'P' },
  { "wide-open", no_argument, 0, 'w' },
  { "syslog", no_argument, 0, 's' },
  { 0, 0, 0, 0 }
};

/* display usage message and terminate */
static void help(void) {
  xprintf("Usage:\n"
	  "  disorderd [OPTIONS]\n"
	  "Options:\n"
	  "  --help, -h               Display usage message\n"
	  "  --version, -V            Display version number\n"
	  "  --config PATH, -c PATH   Set configuration file\n"
	  "  --debug, -d              Turn on debugging\n"
	  "  --foreground, -f         Do not become a daemon\n"
	  "  --syslog, -s             Log to syslog even with -f\n"
	  "  --pidfile PATH, -P PATH  Leave a pidfile\n");
  xfclose(stdout);
  exit(0);
}

/* signals ------------------------------------------------------------------ */

/* SIGHUP callback */
static int handle_sighup(ev_source attribute((unused)) *ev_,
			 int attribute((unused)) sig,
			 void attribute((unused)) *u) {
  info("received SIGHUP");
  reconfigure(ev, 1);
  return 0;
}

/* fatal signals */

static int handle_sigint(ev_source attribute((unused)) *ev_,
			 int attribute((unused)) sig,
			 void attribute((unused)) *u) {
  info("received SIGINT");
  quit(ev);
}

static int handle_sigterm(ev_source attribute((unused)) *ev_,
			  int attribute((unused)) sig,
			  void attribute((unused)) *u) {
  info("received SIGTERM");
  quit(ev);
}

/* periodic actions --------------------------------------------------------- */

struct periodic_data {
  void (*callback)(ev_source *);
  int period;
};

static int periodic_callback(ev_source *ev_,
			     const struct timeval attribute((unused)) *now,
			     void *u) {
  struct timeval w;
  struct periodic_data *const pd = u;

  pd->callback(ev_);
  gettimeofday(&w, 0);
  w.tv_sec += pd->period;
  ev_timeout(ev, 0, &w, periodic_callback, pd);
  return 0;
}

/** @brief Create a periodic action
 * @param ev Event loop
 * @param callback Callback function
 * @param period Interval between calls in seconds
 * @param immediate If true, call @p callback straight away
 */
static void create_periodic(ev_source *ev_,
			    void (*callback)(ev_source *),
			    int period,
			    int immediate) {
  struct timeval w;
  struct periodic_data *const pd = xmalloc(sizeof *pd);

  pd->callback = callback;
  pd->period = period;
  if(immediate)
    callback(ev_);
  gettimeofday(&w, 0);
  w.tv_sec += period;
  ev_timeout(ev_, 0, &w, periodic_callback, pd);
}

static void periodic_rescan(ev_source *ev_) {
  trackdb_rescan(ev_, 1/*check*/);
}

static void periodic_database_gc(ev_source attribute((unused)) *ev_) {
  trackdb_gc();
}

static void periodic_volume_check(ev_source attribute((unused)) *ev_) {
  int l, r;
  char lb[32], rb[32];

  if(!mixer_control(-1/*as configured*/, &l, &r, 0)) {
    if(l != volume_left || r != volume_right) {
      volume_left = l;
      volume_right = r;
      snprintf(lb, sizeof lb, "%d", l);
      snprintf(rb, sizeof rb, "%d", r);
      eventlog("volume", lb, rb, (char *)0);
    }
  }
}

static void periodic_play_check(ev_source *ev_) {
  play(ev_);
}

static void periodic_add_random(ev_source *ev_) {
  add_random_track(ev_);
}

/* We fix the path to include the bindir and sbindir we were installed into */
static void fix_path(void) {
  char *path = getenv("PATH");
  static char *newpath;
  /* static or libgc collects it! */

  if(!path)
    error(0, "PATH is not set at all!");

  if(*finkbindir && strcmp(finkbindir, "/"))
    /* We appear to be a finkized mac; include fink on the path in case the
     * tools we need are there. */
    byte_xasprintf(&newpath, "PATH=%s:%s:%s:%s", 
		   path, bindir, sbindir, finkbindir);
  else
    byte_xasprintf(&newpath, "PATH=%s:%s:%s", path, bindir, sbindir);
  putenv(newpath);
  info("%s", newpath); 
}

int main(int argc, char **argv) {
  int n, background = 1, logsyslog = 0;
  const char *pidfile = 0;

  set_progname(argv);
  mem_init();
  if(!setlocale(LC_CTYPE, "")) fatal(errno, "error calling setlocale");
  /* garbage-collect PCRE's memory */
  pcre_malloc = xmalloc;
  pcre_free = xfree;
  while((n = getopt_long(argc, argv, "hVc:dfP:Ns", options, 0)) >= 0) {
    switch(n) {
    case 'h': help();
    case 'V': version("disorderd");
    case 'c': configfile = optarg; break;
    case 'd': debugging = 1; break;
    case 'f': background = 0; break;
    case 'P': pidfile = optarg; break;
    case 's': logsyslog = 1; break;
    case 'w': wideopen = 1; break;
    default: fatal(0, "invalid option");
    }
  }
  /* go into background if necessary */
  if(background)
    daemonize(progname, LOG_DAEMON, pidfile);
  else if(logsyslog) {
    /* If we're running under some kind of daemon supervisor then we may want
     * to log to syslog but not to go into background */
    openlog(progname, LOG_PID, LOG_DAEMON);
    log_default = &log_syslog;
  }
  info("process ID %lu", (unsigned long)getpid());
  fix_path();
  srand(time(0));			/* don't start the same every time */
  /* gcrypt initialization */
  gcry_control(GCRYCTL_INIT_SECMEM, 1);
  /* create event loop */
  ev = ev_new();
  if(ev_child_setup(ev)) fatal(0, "ev_child_setup failed");
  /* read config */
  if(config_read(1))
    fatal(0, "cannot read configuration");
  /* make sure the home directory exists and has suitable permissions */
  make_home();
  /* Start the speaker process (as root! - so it can choose its nice value) */
  speaker_setup(ev);
  /* set server nice value _after_ starting the speaker, so that they
   * are independently niceable */
  xnice(config->nice_server);
  /* change user */
  become_mortal();
  /* make sure we're not root, whatever the config says */
  if(getuid() == 0 || geteuid() == 0) fatal(0, "do not run as root");
  /* open a lockfile - we only want one copy of the server to run at once. */
  if(config->lock) {
    const char *lockfile;
    int lockfd;
    struct flock lock;

    lockfile = config_get_file("lock");
    if((lockfd = open(lockfile, O_RDWR|O_CREAT, 0600)) < 0)
      fatal(errno, "error opening %s", lockfile);
    cloexec(lockfd);
    memset(&lock, 0, sizeof lock);
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if(fcntl(lockfd, F_SETLK, &lock) < 0)
      fatal(errno, "error locking %s", lockfile);
  }
  /* initialize database environment */
  trackdb_init(TRACKDB_NORMAL_RECOVER|TRACKDB_MAY_CREATE);
  trackdb_master(ev);
  /* install new config (calls trackdb_open()) */
  reconfigure(ev, 0);
  /* pull in old users */
  trackdb_old_users();
  /* create a root login */
  trackdb_create_root();
  /* re-read config if we receive a SIGHUP */
  if(ev_signal(ev, SIGHUP, handle_sighup, 0)) fatal(0, "ev_signal failed");
  /* exit on SIGINT/SIGTERM */
  if(ev_signal(ev, SIGINT, handle_sigint, 0)) fatal(0, "ev_signal failed");
  if(ev_signal(ev, SIGTERM, handle_sigterm, 0)) fatal(0, "ev_signal failed");
  /* ignore SIGPIPE */
  signal(SIGPIPE, SIG_IGN);
  /* Rescan immediately and then daily */
  create_periodic(ev, periodic_rescan, 86400, 1/*immediate*/);
  /* Tidy up the database once a minute */
  create_periodic(ev, periodic_database_gc, 60, 0);
  /* Check the volume immediately and then once a minute */
  create_periodic(ev, periodic_volume_check, 60, 1);
  /* Check for a playable track once a second */
  create_periodic(ev, periodic_play_check, 1, 0);
  /* Try adding a random track immediately and once every two seconds */
  create_periodic(ev, periodic_add_random, 2, 1);
  /* enter the event loop */
  n = ev_run(ev);
  /* if we exit the event loop, something must have gone wrong */
  fatal(errno, "ev_run returned %d", n);
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
End:
*/
