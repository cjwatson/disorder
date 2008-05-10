/*
 * This file is part of DisOrder.
 * Copyright (C) 2008 Richard Kettlewell
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
/** @file server/macros-disorder.h
 * @brief DisOrder-specific expansions
 */

#ifndef MACROS_DISORDER_H
#define MACROS_DISORDER_H

extern disorder_client *client;
extern char *error_string;
void register_disorder_expansions(void);

#define DC_QUEUE 0x0001
#define DC_PLAYING 0x0002
#define DC_RECENT 0x0004
#define DC_VOLUME 0x0008
#define DC_DIRS 0x0010
#define DC_FILES 0x0020
#define DC_NEW 0x0040
#define DC_RIGHTS 0x0080

static struct queue_entry *queue;
static struct queue_entry *playing;
static struct queue_entry *recent;

static int volume_left;
static int volume_right;

static char **files;
static int nfiles;

static char **dirs;
static int ndirs;

static char **new;
static int nnew;

static rights_type rights;

#endif /* MACROS_DISORDER_H */

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
