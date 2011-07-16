#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_fv.py,v 1.7 2011-07-16 17:02:27 grahn Exp $
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


class Synonymous(object):
    """Takes a sequence of sequences of strings, and turns it into
    case-insensitive synonyms.  E.g. [['foo', 'bar', 'baz']] makes
    'foo', 'FOO', 'bAr' and 'baz' synonyms, with 'foo' being the
    canonical form.
    """
    def __init__(self, *v):
        self._v = {}
        for syn in v:
            canon = syn[0]
            for s in syn:
                self._v[s.lower()] = canon
    def get(self, s, default=None):
        return self._v.get(s.lower(), default)


class Record(object):
    """A single record, which gets appended to and finally converted and printed.
    """
    _official = ('artnamn', 'antal', 'enhet', 'antal substrat',
                 'stadium', 'lokalnamn',
                 'nordkoordinat', 'ostkoordinat', 'noggrannhet',
                 'startdatum', 'slutdatum',
                 'kommentar', 'det/conf', 'samling', 'accessionsnr',
                 'substrat - lista', 'substrat - text',
                 'biotop - lista', 'biotop - text',
                 'trädslag - lista', 'trädslag - text',
                 'ej återfunnen', 'andrahandsuppgift', 'osäker bestämning',
                 'utplanterad eller införd', 'intressant notering', 'dölj', 'skydda lokalangivelse',
                 'rapportera till rrk', 'ej funnen', 'undersökt i mikroskop',
                 'syfte', 'floraväktarlokal', 'medobs')
    _synonym = Synonymous(['nordkoordinat', 'nordkoordinat/latitud'],
                          ['ostkoordinat', 'ostkoordinat/longitud'],
                          ['medobs', 'medobservatör'])
    _extension = ('koordinat', 'substrat', 'biotop', 'trädslag', 'obsid')
    _repeatable = ('medobs', )

    _enhet = Synonymous(['buskar', 'buske'],
                        ['m2', 'm²'],
                        ['plantor', 'planta'],
                        ['träd'],
                        ['skott'],
                        ['strån', 'strå'],
                        ['tuvor', 'tuva'],
                        ['stjälkar', 'stjälk'],
                        ['bladskivor', 'bladskiva'],
                        ['stänglar', 'stängel'],
                        ['dm2', 'dm²'],
                        ['cm2', 'cm²'],
                        ['kapslar', 'kapsel'],
                        ['kolonier', 'koloni'],
                        ['bålar', 'bål'],
                        ['mycel'],
                        ['fruktkroppar', 'fruktkropp'])

    class Re:
        """Just a container for some useful REs.
        """
        antal = re.compile(r'(\d+)\s*(.*)')
        noggrannhet = re.compile(r'(\d+) m')
        rt90 = re.compile(r'([67]\d+)\s+(1[2-8]\d+)')

    def __init__(self, log):
        self._log = log
        self._v = {}
        self._cont = None

    def append(self, lineno, field, val):
        """Shovel a new 'field: val' into the record. The field name is trimmed,
        lowercased and complete already; 'val' is trimmed and may be appended to later.
        """
        self._cont = None
        field = self._synonym.get(field, field)
        if field not in self._official and field not in self._extension:
            self._log('warning: line %d: skipping unknown field "%s"\n' % (lineno, field))
            return
        # Early lazy exit for empty fields. Skips a few warnings though. XXX.
        if not val:
            return
        v = self._v
        if field in self._repeatable:
            if v.has_key(field):
                v[field].append(val)
            else:
                v[field] = [val]
            self._cont = field
        else:
            if v.has_key(field):
                self._log('warning: line %d: skipping duplicate field "%s"\n' % (lineno, field))
                return
            v[field] = val
            self._cont = field

    def continuation(self, lineno, val):
        "more data for the last append()ed field"
        val = ' ' + val
        cont = self._cont
        if not cont:
            self._log('warning: line %d: unexpected continuation line skipped\n' % lineno)
            return
        if cont in self._repeatable:
            self._v[cont][-1] += val
        else:
            self._v[cont] += val

    def canonize(self):
        v = self._v

        # canonize antal/enhet
        if not v.has_key('enhet'):
            m = self.Re.antal.match(v.get('antal', ''))
            if m:
                v['antal'] = m.group(1)
                v['enhet'] = m.group(2)
        if v.has_key('enhet'):
            v['enhet'] = self._enhet.get(v['enhet'])

        # canonize coordinates
        k = 'koordinat'
        if v.has_key('nordkoordinat') or v.has_key('ostkoordinat') or v.has_key('noggrannhet'):
            if v.has_key(k):
                self._log('warning: ignoring superfluous "%s: %s"\n' % (k, v[k]))
        elif not v.has_key(k):
            self._log('warning: no coordinate info present\n')
        else:
            m = self.Re.rt90.match(v[k])
            if not m:
                self._log('warning: bad RT90 coordinate %s\n' % v[k])
            else:
                p = Point(int(m.group(1)), int(m.group(2)))
                v['nordkoordinat'] = '%d' % p.north
                v['ostkoordinat'] =  '%d' % p.east
                v['noggrannhet'] =  '%d m' % p.resolution

        # canonize dates
        if v.has_key('startdatum') and not v.has_key('slutdatum'):
            v['slutdatum'] = v['startdatum']

        # canonize the binary fields
        for k in ('ej återfunnen', 'andrahandsuppgift', 'osäker bestämning',
                  'utplanterad eller införd', 'intressant notering', 'dölj', 'skydda lokalangivelse',
                  'rapportera till rrk', 'ej funnen', 'undersökt i mikroskop'):
            if v.get(k):
                v[k] = 'X'

    def dump(self, out):
        self.canonize()
        for k in self._official:
            if k in self._repeatable:
                continue
            s = self._v.get(k)
            if s:
                out.write('%-10s: %s\n' % (k, s))
        for k in self._repeatable:
            v = self._v.get(k, [])
            for s in v:
                out.write('%-10s: %s\n' % (k, s))
        out.write('\n')


def process(f, out, log):
    """Handle one input file 'f', writing to 'out' and logging errors to 'log'.
    """
    r = Record(log)
    line = 0
    for s in f:
        line += 1
        s = s.rstrip()
        if not s:
            r.dump(out)
            r = Record(log)
        elif s[0] == '#':
            continue
        elif s[0] in ' \t':
            r.continuation(line, s.lstrip())
        else:
            field, val = s.split(':', 1)
            field = field.lower().strip()
            val = val.lstrip()
            r.append(line, field, val)
    r.dump(out)


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
