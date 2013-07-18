#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_fv.py,v 1.12 2011-07-16 21:36:10 grahn Exp $
#
# Copyright (c) 2011, 2013 Jörgen Grahn
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
    _official = ('artnamn', 'antal', 'enhet',
                 'ålder-stadium', 'kön', 'lokalnamn',
                 'ost', 'nord', 'noggrannhet',
                 'startdatum', 'slutdatum',
                 'starttid', 'sluttid',
                 'publik kommentar', 'intressant notering', 'privat kommentar',
                 'ej återfunnen',
                 'dölj fyndet t.o.m.',
                 'andrahand', 'osäker artbestämning', 'ospontan',
                 'biotop', 'biotop-beskrivning',
                 'art som substrat', 'art som substrat beskrivning',
                 'substrat', 'substrat-beskrivning',
                 'offentlig samling', 'privat samling', 'samlings-nummer',
                 'artbestämd av', 'bestämningsdatum',
                 'beskrivning artbestämning', 'bekräftad av',
                 'med-observatör')
    _synonym = Synonymous(['ålder-stadium', 'stadium'],
                          ['ost', 'ostkoordinat', 'ostkoordinat/longitud'],
                          ['nord', 'nordkoordinat', 'nordkoordinat/latitud'],
                          ['publik kommentar', 'kommentar'],
                          ['ej återfunnen', 'ej funnen'],
                          ['andrahand', 'andrahandsuppgift'],
                          ['osäker artbestämning', 'osäker bestämning'],
                          ['ospontan', 'utplanterad eller införd'],
                          ['biotop', 'biotop - lista'],
                          ['biotop-beskrivning', 'biotopbeskrivning', 'biotop - text'],
                          ['art som substrat', 'trädslag - lista', 'trädslag'],
                          ['art som substrat beskrivning', 'trädslag - text'],
                          ['substrat', 'substrat - lista'],
                          ['substrat-beskrivning', 'substratbeskrivning', 'substrat - text'],
                          ['samlings-nummer', 'samlingsnummer', 'accessionsnr'],
                          ['artbestämd av', 'det/conf'],
                          ['med-observatör', 'medobs', 'medobservatör'])
    _extension = ('koordinat',
                  'dölj', 'skydda lokalangivelse',
                  'antal substrat', 'samling',
                  'rapportera till rrk', 'undersökt i mikroskop', 'syfte',
                  'floraväktarlokal', 'obsid')
    _unsupptd  = ('dölj', 'skydda lokalangivelse',
                  'antal substrat', 'samling',
                  'rapportera till rrk', 'undersökt i mikroskop', 'syfte',
                  'floraväktarlokal', 'obsid')
    _binary    = ('ej återfunnen', 'andrahand', 'osäker artbestämning',
                  'ospontan', 'intressant notering', 'dölj', 'skydda lokalangivelse',
                  'rapportera till rrk', 'undersökt i mikroskop')
    _repeatable = ('med-observatör', )

    _enhet = Synonymous(['plantor/tuvor', 'plantor', 'tuvor', 'buskar', 'träd',
                         'planta', 'tuva', 'buske'],
                        ['stjälkar/strån', 'stjälkar', 'strån', 'stänglar', 'skott',
                         'stjälk', 'strå', 'stängel'],
                        ['m2', 'm²'],
                        ['dm2', 'dm²'],
                        ['cm2', 'cm²'])
    _substrat = Synonymous()
    _biotop = Synonymous()
    _tree = Synonymous()
    _noggrannhet = Synonymous(['5 m', '5m'],
                              ['10 m', '10m'], ['25 m', '25m'],
                              ['50 m', '50m'], ['75 m', '75m'],
                              ['100 m', '100m'], ['125 m', '125m'], ['150 m', '150m'],
                              ['200 m', '200m'], ['250 m', '250m'], ['300 m', '300m'],
                              ['400 m', '400m'], ['500 m', '500m'], ['750 m', '750m'],
                              ['1000 m', '1000m'], ['1500 m', '1500m'],
                              ['2000 m', '2000m'], ['2500 m', '2500m'],
                              ['3000 m', '3000m'], ['5000 m', '5000m'])
    _stadium = Synonymous(['Vilstadium'],
                          ['Knoppbristning'],
                          ['Fullt utvecklade blad'],
                          ['Blomknopp'],
                          ['Blomning'],
                          ['Överblommad'],
                          ['I frukt'],
                          ['Frukt-/fröspridning'],
                          ['Gulnande löv/blad'],
                          ['Bladfällning, vissnar'],
                          ['Vinterståndare'])
    _samling = Synonymous(['Eget'],
                          ['Annat'],
                          ['Lund-Botaniska/Zoologiska museet'],
                          ['Göteborgs naturhistoriska museum (GNM)'],
                          ['Göteborg-Herbarium GB'],
                          ['Stockholm-Naturhistoriska riksmuseet'],
                          ['Uppsala-Evolutionsmuseet'],
                          ['Uppsala-SLU'],
                          ['Umeå-Herbarium UME'],
                          ['Oskarshamn-Herbarium OHN'],
                          ['Visby-Herbarium VI'],
                          ['Örebro-Herbarium OREB'],
                          ['Lycksele skogsmuseum'],
                          ['Regionmuseet Kristianstad'],
                          ['Helsingfors'],
                          ['Köpenhamn'],
                          ['Oslo'])

    class Re:
        """Just a container for some useful REs.
        """
        antal = re.compile(r'(\d+)\s*(.*)')
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
        k = 'enhet'
        if not v.has_key(k):
            m = self.Re.antal.match(v.get('antal', ''))
            if m:
                v['antal'] = m.group(1)
                v[k] = m.group(2)
        if v.has_key(k):
            s = self._enhet.get(v[k])
            if not s:
                self._log.warning('%s is not a valid choice for field "%s"' % (v[k], k))
            else:
                v[k] = s

        # canonize coordinates
        k = 'koordinat'
        if v.has_key('nord') or v.has_key('ost') or v.has_key('noggrannhet'):
            if v.has_key(k):
                self._log.warning('ignoring superfluous "%s: %s"' % (k, v[k]))
        elif not v.has_key(k):
            if not v.has_key('floraväktarlokal'):
                self._log.warning('no coordinate info present')
        else:
            m = self.Re.rt90.match(v[k])
            if not m:
                self._log.warning('bad RT90 coordinate %s' % v[k])
            else:
                p = Point(int(m.group(1)), int(m.group(2)))
                v['nord'] = '%d' % p.north
                v['ost'] =  '%d' % p.east
                v['noggrannhet'] =  '%d m' % p.resolution

        # canonize time ranges
        for a, b in (('startdatum', 'slutdatum'),
                     ('starttid', 'sluttid')):
            if v.has_key(a) and not v.has_key(b):
                v[b] = v[a]

        # canonize various multi-selection fields
        self._canonize('ålder-stadium', self._stadium)
        self._canonize('noggrannhet', self._noggrannhet)
        self._canonize('samling', self._samling)

        # canonize those crazy descriptive fields
        self._canonize_trinity('substrat - lista', 'substrat - text', 'substrat', self._substrat)
        self._canonize_trinity('biotop - lista', 'biotop - text', 'biotop', self._biotop)
        self._canonize_trinity('trädslag - lista', 'trädslag - text', 'trädslag', self._tree)

        # canonize the binary fields
        for k in self._binary:
            if v.get(k):
                v[k] = 'X'

        # clean away stuff incompatible with floraväktarlokal
        if False and v.has_key('floraväktarlokal'):
            for k in ('lokalnamn',
                      'nordkoordinat', 'ostkoordinat',
                      'noggrannhet'):
                if v.has_key(k): del v[k]

        # warn for mandatory fields missing
        for k in ('artnamn', 'startdatum', 'slutdatum'):
            if not v.has_key(k):
                self._log.warning('mandatory field "%s" missing' % k)
        if False and v.has_key('floraväktarlokal'):
            return
        for k in ('lokalnamn', 'nord', 'ost', 'noggrannhet'):
            if not v.has_key(k):
                self._log.warning('mandatory field "%s" missing' % k)

    def _canonize(self, field, synonyms):
        "helper"
        s = self._v.get(field)
        if s:
            ss = synonyms.get(s)
            if not ss:
                self._log.warning('%s is not a valid choice for field "%s"' % (s, field))
            else:
                self._v[field] = ss

    def _canonize_trinity(self, list, text, both, synonyms):
        """Helper for the substrat/biotop/trädslag fields
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
                v[text] = s

    def dump(self, out):
        """Check the record for validity (and log errors if needed),
        then write it to 'out' using different strategies depending on
        'out'.
        """
        if not self._v:
            # normal case, do nothing
            return
        self.canonize()
        for k in self._unsupptd:
            v = self._v.get(k)
            if v:
                self._log.warning('the field "%s" is supported but '
                                  'will not be converted. ("%s: %s")' % (k, k, v))
        if out.by_field:
            for k in self._official:
                if k in self._repeatable:
                    continue
                out.write(k, self._v.get(k, ''))
            for k in self._repeatable:
                v = self._v.get(k, [])
                for s in v:
                    out.write(k, s)
            out.end()
        else:
            if out.wants_heading():
                out.write(self._official)
            acc = [ self._v.get(x, '') for x in self._official
                    if x not in self._repeatable]
            for k in self._repeatable:
                v = self._v.get(k, [])
                acc += v
            out.write(acc)


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


class DebugOut(object):
    """Output records as "heading: value" lines,
    using a write(key, value) and end() interface.
    """
    by_field = True
    def __init__(self, f):
        self._w = f.write
        self._records_written = 0
        self._at_bor = True
    def write(self, key, val):
        if self._at_bor and self._records_written:
            self._w('\n')
            self._at_bor = False
        if val:
            self._w('%-20s: %s\n' % (key, val))
    def end(self):
        self._at_bor = True
        self._records_written += 1


class CSVOut(object):
    """Output in the intended TAB-separated values format,
    using a write(sequence of values) interface.
    Furthermore, you can ask it if it wants a table heading
    written, since Artportalen needs one of those.

    Assumes, of course, that the caller knows to write() in
    the exact right sequence.
    """
    by_field = False
    def __init__(self, f):
        self._w = f.write
        self._records_written = 0
    def wants_heading(self):
        return self._records_written==0
    def write(self, vals):
        s = '\t'.join(vals)
        self._w('%s\n' % s)
        self._records_written += 1


if __name__ == "__main__":
    import getopt
    import os.path

    prog = os.path.split(sys.argv[0])[1]
    usage = 'usage: %s [-d] file ...' % prog

    out = CSVOut(sys.stdout)

    try:
        opts, files = getopt.getopt(sys.argv[1:], 'd',
                                    ['version', 'help'])
        for opt, val in opts:
            if opt=='-d':
                out = DebugOut(sys.stdout)
            elif opt=='--help':
                print usage
                sys.exit(0)
            elif opt=='--version':
                groblad, ver = cvs_says()
                print '%s, part of %s %s' % (prog, groblad, ver)
                print 'Copyright (c) 2011, 2013 Jörgen Grahn. All rights reserved.'
                sys.exit(0)
    except getopt.GetoptError, s:
        print >>sys.stderr, s
        print >>sys.stderr, usage
        sys.exit(1)

    for f in files:
        process(open(f, 'r'), out, sys.stderr)
    if not files:
        process(sys.stdin, out, sys.stderr)
