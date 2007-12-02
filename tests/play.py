#! /usr/bin/env python
#
# This file is part of DisOrder.
# Copyright (C) 2007 Richard Kettlewell
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
#
import dtest,time,disorder,re

def test():
    """Play some tracks"""
    dtest.start_daemon()
    c = disorder.client()
    track = u"%s/Joe Bloggs/First Album/02:Second track.ogg" % dtest.tracks
    print "adding track to queue"
    c.play(track)
    print "checking track turned up in queue"
    q = c.queue()
    ts = filter(lambda t: t['track'] == track and 'submitter' in t, q)
    assert len(ts) == 1
    t = ts[0]
    assert t['submitter'] == u'fred', "check queue submitter"
    i = t['id']
    print "waiting for track to play"
    p = c.playing()
    while p is None or p['id'] != i:
        time.sleep(1)
        p = c.playing()
    print "waiting for track to finish"
    p = c.playing()
    while p is not None and p['id'] == i:
        time.sleep(1)
        p = c.playing()
    print "checking track turned up in recent list"
    q = c.recent()
    ts = filter(lambda t: t['track'] == track and 'submitter' in t, q)
    assert len(ts) == 1
    t = ts[0]
    assert t['submitter'] == u'fred', "check recent entry submitter"
    
        

if __name__ == '__main__':
    dtest.run()
