#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
# $Id: groblad_report.py,v 1.2 2005-01-06 19:38:37 grahn Exp $
#
# Copyright (c) 2004 Jörgen Grahn <jgrahn@algonet.se>
# All rights reserved.
#

import re
import os
import os.path
import getopt
import sys
import md5
import fileinput
from geese import segrid

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
        self.observers = []
        self.plants = {}
    def contains(self, species):
        return self.plants.has_key(species)

lines = []
headerre = re.compile(r'^(\w+)\s*:\s*(.+)')
plantre1 = re.compile(r'^(.+?)\s*:\S:\s*(.*)')
plantre2 = re.compile(r'^(.+?)\s*:.:\s*(.+)')

for s in fileinput.input():
    s = s.rstrip()
    if s=='': continue
    if s[0]=='#': continue
    if s[0] in ' \t':
        lines[-1] = lines[-1] + ' ' + s.strip()
    else:
        lines.append(s)

places = []
place = None
plants = None
seen = {}

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
            place.coordinate = apply(segrid.Point,
                                     map(int,
                                         value.split()))
        elif name=='date':
            place.date = value
        elif name=='observers':
            place.observers = value.split()
        elif name=='comments':
            pass
    else:
        pass

species = []
speciesre = re.compile(r'^(.+?)\s*\((.+)\)')
f = open('/usr/local/lib/groblad/species', 'r')
for s in f.readlines():
    m = speciesre.match(s)
    if m:
        trivial, latin = m.groups()
        species.append(Species(trivial, latin))

for sp in species:
    name = sp.trivial
    if not seen.has_key(name): continue
    del seen[name]
    print '.XP'
    print sp
    print ':'
    for p in places:
        if not p.contains(name): continue
        if p.coordinate:
            print '%s (%s).' % (p.place, p.coordinate.tstr())
        else:
            print '%s.' % p.place
        if p.plants[name] != '':
            print '%s.' % p.plants[name] 
        print r'\(em'
    print '.'

# At this point 'seen' may contain a number of names
# which aren't good species. Print them as a warning.

for name in seen.keys():
    print >>sys.stderr, 'unknown species:', name
