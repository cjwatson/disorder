/*
 * This file is part of DisOrder.
 * Copyright (C) 2004, 2007, 2008 Richard Kettlewell
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

#ifndef STATE_H
#define STATE_H

void quit(ev_source *ev) attribute((noreturn));
/* terminate the daemon */

int reconfigure(ev_source *ev, int reload);
/* reconfigure.  If @reload@ is nonzero, update the configuration. */

#endif /* QUIT_H */

/*
Local Variables:
c-basic-offset:2
comment-column:40
End:
*/
