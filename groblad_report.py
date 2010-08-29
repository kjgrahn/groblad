#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_report.py,v 1.27 2010-08-29 09:11:49 grahn Exp $
#
# Copyright (c) 2004, 2005, 2007, 2010 Jörgen Grahn
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

class Alias:
    def __init__(self):
        self._aa = []
    def append(self, name):
        name = r'\b%s\b' % re.escape(name)
        self._aa.append(re.compile(name))
    def unalias(self, s):
        """Remove all aliases from string 's', and
        strip exposed whitespace. A bit crude; doesn't
        remove superfluous commas and stuff like that.
        """
        for a in self._aa:
            s = a.sub('', s)
        return s.strip()

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
        sys.stderr.write(s)

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

def parse_files(names):
    """Parse the files named by sequence 'names'
    and return a tuple: list of Place instances,
    and a dictionary of seen species.
    """
    places = []
    seen = {}

    place = None
    plants = None

    fi = FileInput(names)
    log = fi.err

    class State:
        H = 'header'
        L = 'listing'
        O = 'other'

    state = State.O

    for s in fi.input():
        s = s.rstrip()
        if s=='': continue
        if s[0]=='#': continue

        if s=='{':
            if state is not State.O:
                log('error: unexpected "{"\n')
            else:
                place = Place()
                state = State.H
            continue
        if s=='}':
            if state is not State.L:
                log('error: unexpected "}"\n')
            else:
                places.append(place)
                place = None
                plants = None
                state = State.O
            continue
        if s=='}{':
            if state is not State.H:
                log('error: unexpected "}{"\n')
            else:
                plants = place.plants
                state = State.L
            continue

        if state is State.L:
            pp = [ x.strip() for x in s.split(':', 2) ]
            if len(pp)!=3 or not pp[0]:
                log('error: not a species line "%s"\n' % s)
            else:
                name, mark, comment = pp
                if mark or comment:
                    plants[name] = comment
                    seen[name] = 1
            continue

        if state is State.H:
            pp = [ x.strip() for x in s.split(':', 1) ]
            if len(pp)!=2 or not pp[0]:
                log('error: bad header line "%s"\n' % s)
                continue
            name, value = pp
            if name=='place':
                place.place = value
                if not value:
                    log('error: "place" is a mandatory field\n')
            elif name=='coordinate':
                try:
                    if value:
                        place.coordinate = apply(Point, map(int, value.split()))
                except (ValueError, TypeError):
                    log('bad coordinate %s ignored\n' % value)
            elif name=='date':
                place.date = value
                if not value:
                    log('error: "date" is a mandatory field\n')
            elif name=='observers':
                place.observers = value
            elif name in ('comments', 'status'):
                pass
            else:
                log('warning: unknown header "%s"\n' % name)
            continue

        log('error: unexpected line "%s"\n' % s)

    if state is not State.O:
        log('unexpected end of file in %s\n' % state)

    return places, seen

def use_ms(w, places, seen, _):
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

def use_sundh(w, places, seen, _):
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
                        str(p.coordinate.resolution)]
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

def use_svalan(w, places, seen, aliases, fv=False):
    """Like use_sundh, but a different set of crap and no
    taxonomical ordering.
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
    seen.clear()
    spp = {}
    for sp in the_species():
        spp[sp.trivial] = sp
        spp[sp.latin] = sp
    for p in places:
        observers = ''
        if p.observers:
            observers = aliases.unalias(p.observers)
        for name, desc in p.plants.items():
            if not spp.has_key(name):
                seen[name]=1
                continue
            sp = spp[name]
            row = [sp.trivial, '', '', '', '', p.place]
            if p.coordinate:
                row += [str(p.coordinate.north),
                        str(p.coordinate.east),
                        str(p.coordinate.resolution)]
            else:
                row += ['', '', '']
            row += [p.date, p.date, desc]
            row += [''] * 20
            if fv: row.append('')
            row += [observers]
            row += [''] * 9
            w('\t'.join(row))
            w('\n')
    w('.TE\n')

def use_svalan_fv(w, places, seen, aliases):
    return use_svalan(w, places, seen, aliases, True)


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [--ms | --sundh | --svalan | --fv] [--me name] file ...' % prog

    log = sys.stderr.write
    layout = use_ms
    me = Alias()
    try:
        opts, files = getopt.getopt(sys.argv[1:],
                                    '',
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
            elif opt=='--me': me.append(val)
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    places, seen = parse_files(files)

    w = sys.stdout.write
    layout(w, places, seen, me)

    # At this point 'seen' may contain a number of names
    # which aren't good species. Print them as a warning.

    for name in seen.keys():
        log('unknown species: %s\n' % name)
