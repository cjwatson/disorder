/*
 * This file is part of DisOrder.
 * Copyright (C) 2004, 2005, 2006 Richard Kettlewell
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

#ifndef MEM_H
#define MEM_H

#ifdef NO_MEMORY_ALLOCATION
# error including source file not allowed to perform memory allocation
#endif

#include <stdarg.h>

void mem_init(int gc);
/* initialize memory management.  Set GC to 1 if garbage collection is
 * desired. */

void *xmalloc(size_t);
void *xrealloc(void *, size_t);
void *xcalloc(size_t count, size_t size);
/* As malloc/realloc/calloc, but
 * 1) succeed or call fatal
 * 2) always clear (the unused part of) the new allocation
 * 3) are garbage-collected
 */

void *xmalloc_noptr(size_t);
void *xrealloc_noptr(void *, size_t);
char *xstrdup(const char *);
char *xstrndup(const char *, size_t);
/* As malloc/realloc/strdup, but
 * 1) succeed or call fatal
 * 2) are garbage-collected
 * 3) allocated space must not contain any pointers
 *
 * {xmalloc,xrealloc}_noptr don't promise to clear the new space
 */

void xfree(void *ptr);
/* As free, but calls GC_free instead if gc is enabled */

#endif /* MEM_H */

/*
Local Variables:
c-basic-offset:2
comment-column:40
End:
*/
/* arch-tag:333db053cab9e5ef91a151040d3256fb */
