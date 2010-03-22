#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_grep.py,v 1.3 2010-03-22 22:25:52 grahn Exp $
#
# Copyright (c) 2010 Jörgen Grahn
# All rights reserved.
#

import re
import sys

def match(ss, pp, inv = False):
    for s in ss:
        for p in pp:
            if p(s):
                return not inv
    return inv

def subgrep(f, pat, invert, out):
    begin = re.compile('{\s*$').match
    end = re.compile('}\s*$').match
    buf = []
    # I wish I had Perl's flip-flop operator here ...
    inside = False
    for s in f:
        if begin(s) and not inside:
            # {
            buf = [s]
            inside = True
        elif not inside:
            out.write(s)
        elif not end(s):
            buf.append(s)
        else:
            # }
            buf.append(s)
            inside = False
            if match(buf, pat, invert): out.writelines(buf)

def ggrep(files, pat, invert, out):
    for f in files:
        if f=='-':
            f = sys.stdin
        else:
            f = open(f, 'r')
        subgrep(f, pat, invert, out)


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [-v] -e pattern ... file ...' % prog

    log = sys.stderr.write
    invert = False
    pat = []
    files = []

    try:
        opts, files = getopt.getopt(sys.argv[1:], 've:')
        for opt, val in opts:
            if opt=='-v': invert=True
            elif opt=='-e': pat.append(re.compile(val).search)
        if not pat:
            raise ValueError('no pattern specified')
        if not files:
            files.append('-')
    except (ValueError, getopt.GetoptError), s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    ggrep(files, pat, invert, sys.stdout)
