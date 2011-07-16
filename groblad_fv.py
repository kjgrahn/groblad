#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_fv.py,v 1.2 2011-07-16 14:35:13 grahn Exp $
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


def cvs_says(dollarname='$Name:  $'):
    """Parse a CVS tag name according to the convention
    'product_name-major-minor-subminor-...'
    into a tuple: the name and the dot-delimited version number.
    
    Both become 'unknown' if the CVS tag isn't what we'd like it to be.
    """
    import re
    m = re.match(r'\$'r'Name:\s+(.+?)-(\d+(-\d+)*)\D', dollarname)
    if not m: return ('unknown', 'unknown')
    return m.group(1), m.group(2).replace('-', '.')


class Record(object):
    """A single record, which gets appended to and finally converted and printed.
    """


def process(f, out, log):
    """Handle one input file 'f', writing to 'out' and logging errors to 'log'.
    """
    r = Record(out, log)
    line = 0
    for s in f:
        line += 1
        s = s.rstrip()
        if not s:
            r.dump()
            r = Record(out, log)
        elif s[0] == '#':
            continue
        elif s[0] in ' \t':
            r.continuation(line, s)
        else:
            field, val = s.split(':', 1)
            field = field.lower().strip()
            val = val.lstrip()
            r.append(line, field, val)
    r.dump()


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s file ...' % prog

    log = sys.stderr.write
    try:
        opts, files = getopt.getopt(sys.argv[1:], '',
                                    ['version', 'help'])
        for opt, val in opts:
            if opt=='--help':
                print usage
                sys.exit(0)
            elif opt=='--version':
                groblad, ver = cvs_says()
                print '%s, part of %s %s' % (prog, groblad, ver)
                print 'Copyright (c) 2011 Jörgen Grahn. All rights reserved.'
                sys.exit(0)
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    for f in files:
        process(open(f, 'r'), sys.stdout, log)
    if not f:
        process(sys.stdin, sys.stdout, log)
