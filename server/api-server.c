/*
 * This file is part of DisOrder.
 * Copyright (C) 2004, 2005, 2007, 2008 Richard Kettlewell
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
/** @file server/api-server.c
 * @brief Server API functions
 *
 * These functions are made available to server-side plugins.
 */

#include "disorder-server.h"

int disorder_track_exists(const char *track)  {
  return trackdb_exists(track);
}

const char *disorder_track_get_data(const char *track, const char *key)  {
  return trackdb_get(track, key);
}

int disorder_track_set_data(const char *track,
			    const char *key, const char *value)  {
  return trackdb_set(track, key, value);
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
End:
*/
