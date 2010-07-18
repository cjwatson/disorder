/*
 * This file is part of DisOrder.
 * Copyright (C) 2004-2010 Richard Kettlewell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file lib/configuration.h
 * @brief Configuration file support
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <pcre.h>

#include "speaker-protocol.h"
#include "rights.h"
#include "addr.h"

struct uaudio;

/* Configuration is kept in a @struct config@; the live configuration
 * is always pointed to by @config@.  Values in @config@ are UTF-8 encoded.
 */

/** @brief A list of strings */
struct stringlist {
  /** @brief Number of strings */
  int n;
  /** @brief Array of strings */
  char **s;
};

/** @brief A list of list of strings */
struct stringlistlist {
  /** @brief Number of string lists */
  int n;
  /** @brief Array of string lists */
  struct stringlist *s;
};

/** @brief A collection of tracks */
struct collection {
  /** @brief Module that supports this collection */
  char *module;
  /** @brief Filename encoding */
  char *encoding;
  /** @brief Root directory */
  char *root;
};

/** @brief A list of collections */
struct collectionlist {
  /** @brief Number of collections */
  int n;
  /** @brief Array of collections */
  struct collection *s;
};

struct namepart {
  char *part;				/* part */
  pcre *re;				/* compiled regexp */
  char *res;                            /* regexp as a string */
  char *replace;			/* replacement string */
  char *context;			/* context glob */
  unsigned reflags;			/* regexp flags */
};

struct namepartlist {
  int n;
  struct namepart *s;
};

struct transform {
  char *type;				/* track or dir */
  char *context;			/* sort or choose */
  char *replace;			/* substitution string */
  pcre *re;				/* compiled re */
  unsigned flags;			/* regexp flags */
};

struct transformlist {
  int n;
  struct transform *t;
};

/** @brief System configuration */
struct config {
  /* server config */

  /** @brief Authorization algorithm */
  char *authorization_algorithm;
  
  /** @brief All players */
  struct stringlistlist player;

  /** @brief All tracklength plugins */
  struct stringlistlist tracklength;

  /** @brief Allowed users */
  struct stringlistlist allow;

  /** @brief Scratch tracks */
  struct stringlist scratch;

  /** @brief Gap between tracks in seconds */
  long gap;

  /** @brief Maximum number of recent tracks to record in history */
  long history;

  /** @brief Expiry limit for noticed.db */
  long noticed_history;
  
  /** @brief Trusted users */
  struct stringlist trust;

  /** @brief User for server to run as */
  const char *user;

  /** @brief Nice value for rescan subprocess */
  long nice_rescan;

  /** @brief Paths to search for plugins */
  struct stringlist plugins;

  /** @brief List of stopwords */
  struct stringlist stopword;

  /** @brief List of collections */
  struct collectionlist collection;

  /** @brief Database checkpoint byte limit */
  long checkpoint_kbyte;

  /** @brief Databsase checkpoint minimum */
  long checkpoint_min;

  /** @brief Path to mixer device */
  char *mixer;

  /** @brief Mixer channel to use */
  char *channel;

  long prefsync;			/* preflog sync interval */

  /** @brief Secondary listen address */
  struct netaddress listen;

  /** @brief Alias format string */
  const char *alias;

  /** @brief Enable server locking */
  int lock;

  /** @brief Nice value for server */
  long nice_server;

  /** @brief Nice value for speaker */
  long nice_speaker;

  /** @brief Command execute by speaker to play audio */
  const char *speaker_command;

  /** @brief Pause mode for command backend */
  const char *pause_mode;
  
  /** @brief Target sample format */
  struct stream_header sample_format;

  /** @brief Sox syntax generation */
  long sox_generation;

  /** @brief API used to play sound */
  const char *api;

  /** @brief Maximum size of a playlist */
  long playlist_max;

  /** @brief Maximum lifetime of a playlist lock */
  long playlist_lock_timeout;

  /** @brief Home directory for state files */
  const char *home;

  /** @brief Login username */
  char *username;

  /** @brief Login password */
  char *password;

  /** @brief Address to connect to */
  struct netaddress connect;

  /** @brief Directories to search for web templates */
  struct stringlist templates;

  /** @brief Canonical URL of web interface */
  char *url;

  /** @brief Short display limit */
  long short_display;

  /** @brief Maximum refresh interval for web interface (seconds) */
  long refresh;

  /** @brief Minimum refresh interval for web interface (seconds) */
  long refresh_min;

  /** @brief Facilities restricted to trusted users
   *
   * A bitmap of @ref RESTRICT_SCRATCH, @ref RESTRICT_REMOVE and @ref
   * RESTRICT_MOVE.
   */
  unsigned restrictions;		/* restrictions */
#define RESTRICT_SCRATCH 1		/**< Restrict scratching */
#define RESTRICT_REMOVE 2		/**< Restrict removal */
#define RESTRICT_MOVE 4			/**< Restrict rearrangement */

  /** @brief Target queue length */
  long queue_pad;

  /** @brief Minimum time between a track being played again */
  long replay_min;
  
  struct namepartlist namepart;		/* transformations */

  /** @brief Termination signal for subprocesses */
  int signal;

  /** @brief ALSA output device */
  const char *device;
  struct transformlist transform;	/* path name transformations */

  /** @brief Address to send audio data to */
  struct netaddress broadcast;

  /** @brief Source address for network audio transmission */
  struct netaddress broadcast_from;

  /** @brief RTP delay threshold */
  long rtp_delay_threshold;
  
  /** @brief TTL for multicast packets */
  long multicast_ttl;

  /** @brief Whether to loop back multicast packets */
  int multicast_loop;

  /** @brief Login lifetime in seconds */
  long cookie_login_lifetime;

  /** @brief Signing key lifetime in seconds */
  long cookie_key_lifetime;

  /** @brief Default rights for a new user */
  char *default_rights;

  /** @brief Path to sendmail executable */
  char *sendmail;

  /** @brief SMTP server for sending mail */
  char *smtp_server;

  /** @brief Origin address for outbound mail */
  char *mail_sender;

  /** @brief Maximum number of tracks in response to 'new' */
  long new_max;

  /** @brief Minimum interval between password reminder emails */
  long reminder_interval;

  /** @brief Whether to allow user management over TCP */
  int remote_userman;

  /** @brief Maximum age of biased-up tracks */
  long new_bias_age;

  /** @brief Maximum bias */
  long new_bias;

  /** @brief Rescan on (un)mount */
  int mount_rescan;

  /* derived values: */
  int nparts;				/* number of distinct name parts */
  char **parts;				/* name part list  */

  /* undocumented, for testing only */
  long dbversion;
};

extern struct config *config;
/* the current configuration */

int config_read(int server,
                const struct config *oldconfig);
/* re-read config, return 0 on success or non-0 on error.
 * Only updates @config@ if the new configuration is valid. */

char *config_get_file2(struct config *c, const char *name);
char *config_get_file(const char *name);
/* get a filename within the home directory */

struct passwd;

char *config_userconf(const char *home, const struct passwd *pw);
/* get the user's own private conffile, assuming their home dir is
 * @home@ if not null and using @pw@ otherwise */

char *config_usersysconf(const struct passwd *pw );
/* get the user's conffile in /etc */

char *config_private(void);
/* get the private config file */

int config_verify(void);

void config_free(struct config *c);

extern char *configfile;
extern int config_per_user;

extern const struct uaudio *const *config_uaudio_apis;

#endif /* CONFIGURATION_H */

/*
Local Variables:
c-basic-offset:2
comment-column:40
End:
*/
