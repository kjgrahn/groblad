#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_fv.py,v 1.1 2011-07-16 13:22:38 grahn Exp $
#
# Copyright (c) 2011 Jörgen Grahn
# All rights reserved.
#

import re
import sys


class Point:
    """A point in the Rikets Nät.
    """
    def __init__(self, north, east):
        """Create a point based on its north and east values.

        Any input resolution can be used and it is unified here to
        one-metre resolution. Thus, the following are equivalent:

        Point(64457,     13620)
        Point(6445700,   1362000)
        Point(6445700.0, 1362000.0)

        However, the original resolution is remembered for printing
        purposes -- except a 1m resolution is fuzzed to 5m because
        Svalan doesn't accept anything less.
        """
        i = 1
        while north < 1000000:
            i *= 10
            north *= 10
            east *= 10
        self.north = north
        self.east = east
        self.resolution = max(i, 5)
    def __str__(self):
        ne = (self.north / self.resolution,
              self.east / self.resolution)
        return 'Point(%d, %d)' % ne
    def __cmp__(self, other):
        return cmp((self.north, self.east),
                   (other.north, other.east))
    def tstr(self, kind=0):
        """Return the point as a troff(1) source string, with
        point size changing commands as appropriate.
        """
        acc = []
        res = self.resolution
        if res==5: res = 1
        for n in (self.north, self.east):
            n /= res
            n = str(n)
            if kind==0:
                acc.append(r'\s-2%s\s0%s' % (n[:2], n[2:]))
            else:
                acc.append(r'\s-2%s\s0' % n)
        return ' '.join(acc)



if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [--ms | --sundh | --svalan | --fv] [--me name] file ...' % prog

    log = sys.stderr.write
    layout = use_ms
    strict = False
    me = Alias()
    try:
        opts, files = getopt.getopt(sys.argv[1:],
                                    's',
                                    ['ms',
                                     'sundh',
                                     'svalan',
                                     'fv',
                                     'me='])
        for opt, val in opts:
            if opt=='--sundh': layout = use_sundh
            elif opt=='--svalan': layout = use_svalan
            elif opt=='--fv': layout = use_svalan_fv
            elif opt=='--ms': layout = use_ms
            elif opt=='-s': strict = True
            elif opt=='--me': me.append(val)
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    if layout not in (use_svalan, use_svalan_fv):
        strict = True

    places, seen = parse_files(files)

    w = sys.stdout.write
    layout(w, places, seen, me, strict)

    # At this point 'seen' may contain a number of names
    # which aren't good species. Print them as a warning.

    for name in seen.keys():
        log('unknown species: %s\n' % name)
    if seen and not strict:
        log('(unknown species included anyway)\n')
