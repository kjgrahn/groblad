# Makefile
#
# Copyright (c) 2013 Jörgen Grahn
# All rights reserved.

SHELL=/bin/bash

all: groblad
all: groblad_cat
all: groblad_grep
all: build/groblad_report.py
all: build/groblad_fv.py

all: build/groblad.1
all: build/groblad.5
all: groblad_cat.1
all: groblad_grep.1
all: build/groblad_report.1
all: build/groblad_fv.1

all: default
all: species

INSTALLBASE = /usr/local

version.c: Makefile mkversion
	./mkversion groblad_{name=Groblad,version=3.0,prefix=$(INSTALLBASE)} $@

libgavia.a: contstream.o
libgavia.a: files...o
libgavia.a: taxon.o
libgavia.a: taxa.o
libgavia.a: date.o
libgavia.a: excursion.o
libgavia.a: excursion_put.o
libgavia.a: indent.o
libgavia.a: regex.o
libgavia.a: filetest.o
libgavia.a: editor.o
libgavia.a: md5.o
libgavia.a: md5pp.o
libgavia.a: version.o
	$(AR) -r $@ $^

groblad_cat: groblad_cat.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

groblad_grep: groblad_grep.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

groblad: groblad.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

CFLAGS=-W -Wall -pedantic -ansi -g -Os
CXXFLAGS=-W -Wall -pedantic -std=c++98 -g -Os

species_raw: kärlvl
species_raw: dyntaxa/Tracheophyta
	./kärlvl dyntaxa/Tracheophyta | unexpand -a >$@

.PHONY: install
install: all
	install -m555 groblad{,_cat,_grep} $(INSTALLBASE)/bin/
	install -m555 build/groblad_report.py $(INSTALLBASE)/bin/groblad_report
	install -m555 build/groblad_fv.py $(INSTALLBASE)/bin/groblad_fv
	install -m644 build/*.1 $(INSTALLBASE)/man/man1/
	install -m644 build/*.5 $(INSTALLBASE)/man/man5/
	install -d $(INSTALLBASE)/lib/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad

$(INSTALLBASE)/lib/groblad:
	install -d $@

build/%: %
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@

.PHONY: tags
tags: TAGS
TAGS:
	etags *.{c,h,cc,hh}

.PHONY: depend
depend:
	makedepend -- $(CFLAGS) -- -Y -I. *.{c,cc}

.PHONY: clean
clean:
	$(RM) groblad
	$(RM) groblad_cat
	$(RM) groblad_grep
	$(RM) build/groblad_report.py
	$(RM) build/groblad_fv.py
	$(RM) build/*.[15]
	$(RM) *.o lib*.a
	$(RM) *.pyc
	$(RM) version.c
	$(RM) Makefile.bak
	$(RM) species_raw

love:
	@echo "not war?"

# DO NOT DELETE

md5.o: md5.h
contstream.o: contstream.h
date.o: date.h
editor.o: editor.h
excursion.o: excursion.h taxon.h date.h taxa.h lineparse.h files...h
excursion_put.o: excursion.h taxon.h date.h indent.h
files...o: files...h
filetest.o: filetest.h
groblad_cat.o: files...h taxa.h taxon.h excursion.h date.h
groblad.o: taxa.h taxon.h files...h excursion.h date.h lineparse.h editor.h
groblad.o: filetest.h md5pp.h md5.h
groblad_grep.o: files...h taxa.h taxon.h excursion.h date.h regex.h
indent.o: indent.h
md5pp.o: md5pp.h md5.h
regex.o: regex.h
taxa.o: taxa.h taxon.h lineparse.h
taxon.o: taxon.h regex.h
