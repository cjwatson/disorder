/*
 * This file is part of DisOrder
 * Copyright (C) 2008 Richard Kettlewell
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

/** @file lib/bits.h
 * @brief Bit operations
 */

#ifndef BITS_H
#define BITS_H

#if HAVE_FLS
static inline int leftmost_bit(uint32_t n) {
  return fls(n) - 1;
}
#else
int leftmost_bit(uint32_t n);
#endif

#endif /* BITS_H */


/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
