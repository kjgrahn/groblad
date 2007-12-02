#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_report.py,v 1.18 2007-12-02 12:39:56 grahn Exp $
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

class FileInput:
    """Line generator, from a seqyence of file names.
    Similar to fileinput.input, but doesn't suck,
    and handles continuation lines.
    Empty sequence means standard input.
    """
    def __init__(self, names):
        self._names = list(names)
        if not self._names:
            # XXX kludge
            self._names = [sys.stdin]

    def err(self, s):
        """Print an error/warning to stderr, with current
        file/line number prepended
        """
        sys.stderr.write('%s:%d: ' % (self._currfile, self._currline))

    def _yfrags(self):
        s = ' '.join([ f.strip() for f in self._frags ]) + '\n'
        self._frags = []
        return s 

    def input(self):
        for name in self._names:
            if name is sys.stdin:
                f = sys.stdin
            else:
                f = open(name, 'r')
            self._currfile = f.name
            self._currline = 1
            self._frags = []
            for s in f:
                if s.isspace():
                    if self._frags:
                        yield self._yfrags()
                    yield s
                elif s[0].isspace():
                    self._frags.append(s)
                else:
                    if self._frags:
                        yield self._yfrags()
                    self._frags.append(s)
                self._currline += 1
            if self._frags:
                yield self._yfrags()
            if f is not sys.stdin:
                f.close()

def parse_files(log, names):
    """Parse the files named by sequence 'names'
    and return a tuple: list of Place instances,
    and a dictionary of seen species.
    """
    places = []
    seen = {}

    headerre = re.compile(r'^(\w+)\s*:\s*(.+)')
    plantre1 = re.compile(r'^(.+?)\s*:\S:\s*(.*)')
    plantre2 = re.compile(r'^(.+?)\s*:.:\s*(.+)')
    place = None
    plants = None

    for s in FileInput(names).input():
        s = s.rstrip()
        if s=='': continue
        if s[0]=='#': continue

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

def use_sundh(w, places, seen):
    """Like use_ms, but a different layout.
    """
    w('.TS H\n'
      'allbox;\n'
      'l lfI lp4 lp4 lp4 l lp-2 lp-2 l l l l l l l.\n')
    w('\t'.join(['art',
                 '',
                 'förs.',
                 'kommun',
                 'landskap',
                 'lokal',
                 'nord',
                 'ost',
                 'fel',
                 'år',
                 'leg',
                 'nyfynd',
                 'återfynd',
                 'antal',
                 'biotop']))
    w('\n'
      '.TH\n')
    for sp in the_species():
        name = sp.trivial
        if not seen.has_key(name): continue
        del seen[name]
        for p in places:
            if not p.contains(name): continue
            row = [sp.trivial, sp.latin, '', '', '', p.place]
            if p.coordinate:
                row += [str(p.coordinate.north),
                        str(p.coordinate.east),
                        '']
            else:
                row += ['', '', '']
            try:
                year = p.date[:4]
                int(year)
                row += [year]
            except:
                row += ['']
            row += ['', '', '', '', '']
            w('\t'.join(row))
            w('\n')
    w('.TE\n')

def use_svalan(w, places, seen):
    """Like use_sundh, but a different set of crap.
    """
    w('.TS H\n'
      'allbox;\n'
      'lrllllrrrlll llllllllllllllllllllllllllllll.\n')
    w('\t'.join(['art',
                 'antal',
                 'enhet',
                 'antal substrat',
                 'stadium',
                 'lokalnamn',
                 'nord',
                 'ost',
                 'noggrannhet',
                 'startdatum',
                 'slutdatum',
                 'kommentar',

                 'det/conf',
                 'samling',
                 'accessionsnr',
                 'substrat - lista',
                 'substrat - text',
                 'biotop - lista',
                 'biotop - text',
                 'trädslag - lista',
                 'trädslag - text',
                 'ej återfunnen',
                 'andrahandsuppgift',
                 'osäker bestämning',
                 'utplanterad eller införd',
                 'intressant notering',
                 'dölj',
                 'skydda lokalangivelse',
                 'rapportera till Rrk',
                 'ej funnen',
                 'undersökt i mikroskop',
                 'syfte',
                 'medobs 1',
                 'medobs 2',
                 'medobs 3',
                 'medobs 4',
                 'medobs 5',
                 'medobs 6',
                 'medobs 7',
                 'medobs 8',
                 'medobs 9',
                 'medobs 10']))
    w('\n'
      '.TH\n')
    for sp in the_species():
        name = sp.trivial
        if not seen.has_key(name): continue
        del seen[name]
        for p in places:
            if not p.contains(name): continue
            row = [sp.trivial, '', '', '', '', p.place]
            if p.coordinate:
                row += [str(p.coordinate.north),
                        str(p.coordinate.east),
                        '']
            else:
                row += ['', '', '']
            row += [p.date, p.date, p.plants[name]]
            row += [''] * 30
            w('\t'.join(row))
            w('\n')
    w('.TE\n')


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [--ms | --sundh | --svalan] file ...' % prog

    log = sys.stderr.write
    layout = use_ms
    try:
        opts, files = getopt.getopt(sys.argv[1:],
                                    '',
                                    ['ms',
                                     'sundh',
                                     'svalan'])
        for opt, val in opts:
            if opt=='--sundh': layout = use_sundh
            elif opt=='--svalan': layout = use_svalan
            elif opt=='--ms': layout = use_ms
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    places, seen = parse_files(log, files)

    w = sys.stdout.write
    layout(w, places, seen)

    # At this point 'seen' may contain a number of names
    # which aren't good species. Print them as a warning.

    for name in seen.keys():
        log('unknown species: %s\n' % name)
