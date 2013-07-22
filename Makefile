# Makefile
#
# Copyright (c) 2013 Jörgen Grahn
# All rights reserved.

SHELL=/bin/bash

all: groblad
all: groblad_cat
all: groblad_grep
all: _groblad.pl _groblad_report.py
all: _groblad.1 _groblad.5 _groblad_report.1 groblad_grep.1
all: groblad_grep.py
all: groblad_fv.py groblad_fv.1
all: default species

INSTALLBASE = /usr/local

version.c: Makefile mkversion
	./mkversion groblad_{name=Groblad,version=XXX,prefix=$(INSTALLBASE)} $@

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

karlsson/falat%.html: karlsson/falat%.htm
	perl -pe 's/ width=".+?"//g' <$< >$@

falat: karlsson/falat1.html
falat: karlsson/falat1b.html
falat: karlsson/falat2.html
falat: karlsson/falat2b.html
falat: karlsson/falat3.html
falat: karlsson/falat3b.html
falat: karlsson/falat3c.html
falat: karlsson/falat4.html
falat: karlsson/falat5.html
falat: karlsson/falat5b.html
falat: karlsson/falat6.html
falat: karlsson/falat7.html
falat: karlsson/falat7b.html
falat: karlsson/falat8.html
falat: karlsson/falat9.html
falat: karlsson/falat9b.html
falat: karlsson/falat10.html
falat: karlsson/falat11.html
falat: karlsson/falat11b.html
falat: karlsson/falat12.html
falat: karlsson/falat13.html
falat: karlsson/falat14.html
falat: karlsson/falat14b.html
falat: karlsson/falat14c.html
falat: karlsson/falat15.html
falat: karlsson/falat15b.html
falat: karlsson/falat16.html
falat: karlsson/falat17.html
falat: karlsson/falat18.html
falat: karlsson/falat18b.htm
	( for f in $^ ; do w3m -cols 2000 -dump $$f ; done) >$@

species_raw: falat kärlvl
	./kärlvl falat | col >$@

.PHONY: install
install: all
	install -m555 _groblad.pl $(INSTALLBASE)/bin/groblad
	install -m555 _groblad_report.py $(INSTALLBASE)/bin/groblad_report
	install -m555 groblad_grep.py $(INSTALLBASE)/bin/groblad_grep
	install -m555 groblad_fv.py $(INSTALLBASE)/bin/groblad_fv
	install -d $(INSTALLBASE)/lib/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad
	install -m644 _groblad.1 $(INSTALLBASE)/man/man1/groblad.1
	install -m644 _groblad_report.1 $(INSTALLBASE)/man/man1/groblad_report.1
	install -m644 groblad_grep.1 $(INSTALLBASE)/man/man1/
	install -m644 groblad_fv.1 $(INSTALLBASE)/man/man1/
	install -m644 _groblad.5 $(INSTALLBASE)/man/man5/groblad.5

$(INSTALLBASE)/lib/groblad:
	install -d $@

_%: %
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
	$(RM) *.o lib*.a
	$(RM) version.c
	$(RM) Makefile.bak
	$(RM) _groblad.pl _groblad_report.py
	$(RM) _groblad.1 _groblad.5 _groblad_report.1
	$(RM) species_raw
	$(RM) falat
	$(RM) karlsson/falat*.html

love:
	@echo "not war?"

# DO NOT DELETE

md5.o: md5.h
date.o: date.h
editor.o: editor.h
excursion.o: taxa.h taxon.h lineparse.h files...h
excursion_put.o: indent.h
files...o: files...h
filetest.o: filetest.h
indent.o: indent.h
md5pp.o: md5pp.h md5.h
regex.o: ./regex.h
taxa.o: taxa.h taxon.h lineparse.h
taxon.o: taxon.h
