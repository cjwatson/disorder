/*
 * This file is part of DisOrder.
 * Copyright (C) 2005, 2007, 2008 Richard Kettlewell
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
#include "test.h"

void test_url(void) {
  struct url p;
  
  printf("test_url\n");

  insist(parse_url("http://www.example.com/example/path", &p) == 0);
  check_string(p.scheme, "http");
  check_string(p.host, "www.example.com");
  insist(p.port == -1);
  check_string(p.path, "/example/path");
  insist(p.query == 0);

  insist(parse_url("https://www.example.com:82/example%2fpath?+query+", &p) == 0);
  check_string(p.scheme, "https");
  check_string(p.host, "www.example.com");
  insist(p.port == 82);
  check_string(p.path, "/example/path");
  check_string(p.query, "+query+");

  insist(parse_url("//www.example.com/example/path", &p) == 0);
  insist(p.scheme == 0);
  check_string(p.host, "www.example.com");
  insist(p.port == -1);
  check_string(p.path, "/example/path");
  insist(p.query == 0);

  insist(parse_url("http://www.example.com:100000/", &p) == -1);
  insist(parse_url("http://www.example.com:1000000000000/", &p) == -1);
  insist(parse_url("http://www.example.com/example%2zpath", &p) == -1);
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/