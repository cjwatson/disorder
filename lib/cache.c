/*
 * This file is part of DisOrder
 * Copyright (C) 2006 Richard Kettlewell
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

#include <time.h>

#include "hash.h"
#include "mem.h"
#include "log.h"
#include "cache.h"

static hash *h;

struct cache_entry {
  const struct cache_type *type;
  const void *value;
  time_t birth;
};

static int expired(const struct cache_entry *c, time_t now) {
  return now - c->birth > c->type->lifetime;
}

void cache_put(const struct cache_type *type,
               const char *key, const void *value) {
  struct cache_entry *c;
  
  if(!h)
    h = hash_new(sizeof (struct cache_entry));
  c = xmalloc(sizeof *c);
  c->type = type;
  c->value = value;
  time(&c->birth);
  hash_add(h, key, c,  HASH_INSERT_OR_REPLACE);
}

const void *cache_get(const struct cache_type *type, const char *key) {
  const struct cache_entry *c;
  
  if(h
     && (c = hash_find(h, key))
     && c->type == type
     && !expired(c, time(0)))
    return c->value;
  else
    return 0;
}

static int expiry_callback(const char *key, void *value, void *u) {
  const struct cache_entry *c = value;
  const time_t *now = u;
  
  if(expired(c, *now))
    hash_remove(h, key);
  return 0;
}

void cache_expire(void) {
  time_t now;

  if(h) {
    time(&now);
    hash_foreach(h, expiry_callback, &now);
  }
}

static int clean_callback(const char *key, void *value, void *u) {
  const struct cache_entry *c = value;
  const struct cache_type *type = u;

  if(!type || c->type == type)
    hash_remove(h, key);
  return 0;
}

void cache_clean(const struct cache_type *type) {
  if(h)
    hash_foreach(h, clean_callback, (void *)type);
}

size_t cache_count(void) {
  return h ? hash_count(h) : 0;
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
