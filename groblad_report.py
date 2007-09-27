#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_report.py,v 1.12 2007-09-27 21:48:05 grahn Exp $
#
# Copyright (c) 2004, 2005, 2007 Jörgen Grahn <jgrahn@algonet.se>
# All rights reserved.
#

import re
import sys


class Species:
    def __init__(self, trivial, latin):
        self.trivial = trivial
        self.latin = latin
    def __str__(self):
        return r'\fB%s\fP \fI%s\fP' % (self.trivial,
                                       self.latin)

class Place:
    def __init__(self):
        self.place = None
        self.coordinate = None
        self.date = None
        self.observers = None
        self.plants = {}
    def contains(self, species):
        return self.plants.has_key(species)

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
        purposes.
        """
        i = 1
        while north < 1000000:
            i *= 10
            north *= 10
            east *= 10
        self.north = north
        self.east = east
        self.resolution = i
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
        for n in (self.north, self.east):
            n /= self.resolution
            n = str(n)
            if kind==0:
                acc.append(r'\s-2%s\s0%s' % (n[:2], n[2:]))
            else:
                acc.append(r'\s-2%s\s0' % n)
        return ' '.join(acc)

def the_species():
    """Pull the species list from file. Returned as
    an ordered list of Species instances.
    """
    species = []
    speciesre = re.compile(r'^(.+?)\s*\((.+)\)')
    f = open('INSTALLBASE/lib/groblad/species', 'r')
    for s in f.readlines():
        m = speciesre.match(s)
        if m:
            trivial, latin = m.groups()
            species.append(Species(trivial, latin))
    return species

def fileinput(names):
    """Line generator, from a seqyence of file names.
    Similar to fileinput.input, but doesn't suck.
    Empty sequence means standard input.
    """
    did_files = False
    for name in names:
        did_files = True
        f = open(name, 'r')
        for s in f: yield s
    if did_files: return
    for s in sys.stdin: yield s

def parse_files(log, names):
    """Parse the files named by sequence 'names'
    and return a tuple: list of Place instances,
    and a dictionary of seen species.
    """
    lines = []
    for s in fileinput(names):
        s = s.rstrip()
        if s=='': continue
        if s[0]=='#': continue
        if s[0] in ' \t':
            lines[-1] = lines[-1] + ' ' + s.strip()
        else:
            lines.append(s)

    places = []
    seen = {}

    headerre = re.compile(r'^(\w+)\s*:\s*(.+)')
    plantre1 = re.compile(r'^(.+?)\s*:\S:\s*(.*)')
    plantre2 = re.compile(r'^(.+?)\s*:.:\s*(.+)')
    place = None
    plants = None
    for s in lines:
        if s=='{':
            place = Place()
            continue
        if s=='}':
            places.append(place)
            place = None
            plants = None
            continue
        if s=='}{':
            plants = place.plants
            continue
        if plants != None:
            for rex in (plantre1, plantre2):
                m = rex.match(s)
                if m:
                    name, comment = m.groups()
                    plants[name] = comment
                    seen[name] = 1
            continue
        m = headerre.match(s)
        if m:
            name, value = m.groups()
            if name=='place':
                place.place = value
            elif name=='coordinate':
                try:
                    place.coordinate = apply(Point, map(int, value.split()))
                except ValueError:
                    log('bad coordinate %s ignored\n' % value)
            elif name=='date':
                place.date = value
            elif name=='observers':
                place.observers = value
            elif name=='comments':
                pass
            else:
                log('hey, what\'s this? %s\n' % s)
        else:
            pass
    return places, seen

def use_ms(w, places, seen):
    """Layout, writing with 'w' from 'places',
    while consuming 'seen'.
    """
    for sp in the_species():
        name = sp.trivial
        if not seen.has_key(name): continue
        del seen[name]
        w('.XP\n')
        w('%s:\n' % sp)
        ss = []
        for p in places:
            if not p.contains(name): continue
            if p.coordinate:
                s = '%s\n(%s).' % (p.place, p.coordinate.tstr())
            else:
                s = '%s.' % p.place
            if p.date and p.observers:
                s += '\n%s %s.' % (p.observers, p.date)
            if p.plants[name] != '':
                s += '\n%s' % p.plants[name]
            if s[-1] != '.':
                s += '.'
            ss.append(s)
        w('\n\\(em\n'.join(ss))
        w('\n.\n')

if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [--tbl | --sundh] file ...' % prog

    log = sys.stderr.write
    use_sundh = False
    try:
        opts, files = getopt.getopt(sys.argv[1:],
                                    '',
                                    ['tbl',
                                     'sundh'])
        for opt, val in opts:
            use_sundh = (opt=='--sundh')
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    places, seen = parse_files(log, files)

    w = sys.stdout.write
    use_ms(w, places, seen)

    # At this point 'seen' may contain a number of names
    # which aren't good species. Print them as a warning.

    for name in seen.keys():
        log('unknown species: %s\n' % name)
