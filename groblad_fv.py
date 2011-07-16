#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_fv.py,v 1.10 2011-07-16 18:11:45 grahn Exp $
#
# Copyright (c) 2011 J�rgen Grahn
# All rights reserved.
#

import re
import sys


class Point:
    """A point in the Rikets N�t.
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
                 'tr�dslag - lista', 'tr�dslag - text',
                 'ej �terfunnen', 'andrahandsuppgift', 'os�ker best�mning',
                 'utplanterad eller inf�rd', 'intressant notering', 'd�lj', 'skydda lokalangivelse',
                 'rapportera till rrk', 'ej funnen', 'unders�kt i mikroskop',
                 'syfte', 'florav�ktarlokal', 'medobs')
    _synonym = Synonymous(['nordkoordinat', 'nordkoordinat/latitud'],
                          ['ostkoordinat', 'ostkoordinat/longitud'],
                          ['medobs', 'medobservat�r'])
    _extension = ('koordinat', 'substrat', 'biotop', 'tr�dslag', 'obsid')
    _repeatable = ('medobs', )

    _enhet = Synonymous(['buskar', 'buske'],
                        ['m2', 'm�'],
                        ['plantor', 'planta'],
                        ['tr�d'],
                        ['skott'],
                        ['str�n', 'str�'],
                        ['tuvor', 'tuva'],
                        ['stj�lkar', 'stj�lk'],
                        ['bladskivor', 'bladskiva'],
                        ['st�nglar', 'st�ngel'],
                        ['dm2', 'dm�'],
                        ['cm2', 'cm�'],
                        ['kapslar', 'kapsel'],
                        ['kolonier', 'koloni'],
                        ['b�lar', 'b�l'],
                        ['mycel'],
                        ['fruktkroppar', 'fruktkropp'])
    _substrat = Synonymous(['Marken'],
                           ['Brandplats'],
                           ['Sand'],
                           ['Lera'],
                           ['Kalkrik jord'],
                           ['N�ringsrik jord'],
                           ['N�ringsfattig jord'],
                           ['Annan mark'],
                           ['Ved'],
                           ['Levande tr�d'],
                           ['H�gstubbe/d�tt tr�d'],
                           ['L�ga'],
                           ['Stubbe'],
                           ['�vrig d�d ved'],
                           ['Annan ved'],
                           ['Andra v�xter'],
                           ['Levande k�rlv�xter'],
                           ['Kottar/n�tter/ollon'],
                           ['L�vf�rna'],
                           ['Barrf�rna'],
                           ['�vriga v�xtrester'],
                           ['Andra v�xter'],
                           ['Spillning'],
                           ['�lgspillning'],
                           ['R�djursspillning'],
                           ['Hare/kaninspillning'],
                           ['H�stspillning'],
                           ['Kospillning'],
                           ['F�rspillning'],
                           ['Annan djurspillning'])
    _biotop = Synonymous(['Skogsmark'],
                         ['�dell�vskog'],
                         ['Triviall�vskog'],
                         ['Blandskog'],
                         ['Tallskog'],
                         ['Granskog'],
                         ['H�ssle'],
                         ['Hygge'],
                         ['Annan skogsmark'],
                         ['�ngs- och betesmark'],
                         ['�ppen buskmark'],
                         ['Buskb�rande mark'],
                         ['Tr�db�rande mark'],
                         ['Annan �ngs-/betesmark'],
                         ['V�tmark'],
                         ['Rikk�rr'],
                         ['Fattigk�rr'],
                         ['Mosse'],
                         ['Strand'],
                         ['Annan v�tmark'],
                         ['Kalfj�ll'],
                         ['Rik fj�llhed'],
                         ['Fattig fj�llhed'],
                         ['Videsn�r'],
                         ['M�nniskoskapad'],
                         ['�ker'],
                         ['All�'],
                         ['Park/kyrkog�rd/gr�smatta'],
                         ['G�rdsmilj�'],
                         ['Ruderatmark'],
                         ['Annan m�nniskoskapad'])
    _tree = Synonymous(['Barrtr�d'],
                       ['En'],
                       ['Gran'],
                       ['L�rk'],
                       ['Tall'],
                       ['Annat barrtr�d'],
                       ['�dell�vtr�d'],
                       ['Alm'],
                       ['Ask'],
                       ['Avenbok'],
                       ['Bok'],
                       ['Ek'],
                       ['F�gelb�r'],
                       ['Hassel'],
                       ['H�stkastanj'],
                       ['Lind'],
                       ['L�nn'],
                       ['Annat �dell�vtr�d'],
                       ['Triviall�vtr�d'],
                       ['Apel'],
                       ['Asp'],
                       ['Bj�rk'],
                       ['Gr�al/klibbal'],
                       ['Oxel'],
                       ['R�nn'],
                       ['Salix'],
                       ['Annat trivialt l�vtr�d'])

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

    def append(self, field, val):
        """Shovel a new 'field: val' into the record. The field name is trimmed,
        lowercased and complete already; 'val' is trimmed and may be appended to later.
        """
        self._cont = None
        field = self._synonym.get(field, field)
        if field not in self._official and field not in self._extension:
            self._log.warning('skipping unknown field "%s"' % field)
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
                self._log.warning('skipping duplicate field "%s"' % field)
                return
            v[field] = val
            self._cont = field

    def continuation(self, val):
        "more data for the last append()ed field"
        val = ' ' + val
        cont = self._cont
        if not cont:
            self._log.warning('unexpected continuation line skipped')
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
                self._log.warning('ignoring superfluous "%s: %s"' % (k, v[k]))
        elif not v.has_key(k):
            self._log.warning('no coordinate info present')
        else:
            m = self.Re.rt90.match(v[k])
            if not m:
                self._log.warning('bad RT90 coordinate %s' % v[k])
            else:
                p = Point(int(m.group(1)), int(m.group(2)))
                v['nordkoordinat'] = '%d' % p.north
                v['ostkoordinat'] =  '%d' % p.east
                v['noggrannhet'] =  '%d m' % p.resolution

        # canonize dates
        if v.has_key('startdatum') and not v.has_key('slutdatum'):
            v['slutdatum'] = v['startdatum']

        # canonize those crazy descriptive fields
        self._canonize_trinity('substrat - lista', 'substrat - text', 'substrat', self._substrat)
        self._canonize_trinity('biotop - lista', 'biotop - text', 'biotop', self._biotop)
        self._canonize_trinity('tr�dslag - lista', 'tr�dslag - text', 'tr�dslag', self._tree)

        # canonize the binary fields
        for k in ('ej �terfunnen', 'andrahandsuppgift', 'os�ker best�mning',
                  'utplanterad eller inf�rd', 'intressant notering', 'd�lj', 'skydda lokalangivelse',
                  'rapportera till rrk', 'ej funnen', 'unders�kt i mikroskop'):
            if v.get(k):
                v[k] = 'X'

        # clean away stuff incompatibe with florav�ktarlokal
        if v.has_key('florav�ktarlokal'):
            del v['lokalnamn']
            del v['nordkoordinat']
            del v['ostkoordinat']
            del v['noggrannhet']

    def _canonize_trinity(self, list, text, both, synonyms):
        """Helper for the substrat/biotop/tr�dslag fields
        """
        v = self._v
        s = v.get(list)
        if s:
            ss = synonyms.get(s)
            if not ss:
                self._log.warning('%s is not a valid choice for field "%s"' % (s, list))
            else:
                v[list] = ss
        s = v.get(both)
        if s:
            if v.has_key(list) or v.has_key(text):
                self._log.warning('the "%s" field trumps "%s" or "%s"' % (both, list, text))
            ss = synonyms.get(s)
            if ss:
                v[list] = ss
            else:
                v[text] = ss

    def dump(self, out):
        if not self._v:
            # normal case, do nothing
            return
        self.canonize()
        obsid = self._v.get('obsid')
        if obsid:
            self._log.warning('the observation seems to have been reported before, as obsid %s' % obsid)
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


class Log(object):
    """Logging problems (error, warning) with source file and line number to some file.
    """
    def __init__(self, srcname, out):
        self._src = srcname
        self._line = 0
        self._out = out
    def newline(self):
        self._line += 1
    def _msg(self, heading, s):
        self._out.write('%s:%d: %s: %s\n' % (self._src, self._line, heading, s))
    def error(self, s):
        self._msg('error', s)
    def warning(self, s):
        self._msg('warning', s)


def process(f, out, log):
    """Handle one input file 'f', writing to 'out' and logging errors to 'log'.
    """
    log = Log(f.name, log)
    r = Record(log)
    for s in f:
        log.newline()
        s = s.rstrip()
        if not s:
            r.dump(out)
            r = Record(log)
        elif s[0] == '#':
            continue
        elif s[0] in ' \t':
            r.continuation(s.lstrip())
        else:
            field, val = s.split(':', 1)
            field = field.lower().strip()
            val = val.lstrip()
            r.append(field, val)
    r.dump(out)


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s file ...' % prog

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
                print 'Copyright (c) 2011 J�rgen Grahn. All rights reserved.'
                sys.exit(0)
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    for f in files:
        process(open(f, 'r'), sys.stdout, sys.stderr)
    if not files:
        process(sys.stdin, sys.stdout, sys.stderr)
