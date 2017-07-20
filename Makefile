# Makefile
#
# Copyright (c) 2013 Jörgen Grahn
# All rights reserved.

SHELL=/bin/bash

all: groblad
all: groblad_cat
all: groblad_grep
all: groblad_report
all: build/groblad_fv.py

all: build/groblad.1
all: build/groblad.5
all: build/groblad_species.5
all: build/groblad_cat.1
all: build/groblad_grep.1
all: build/groblad_report.1
all: build/groblad_fv.1

all: default
all: species
all: linnaeus.nrm.se

all: test/test

INSTALLBASE = /usr/local

version.c: Makefile mkversion
	./mkversion groblad_{name=Groblad,version=3.8,prefix=$(INSTALLBASE)} $@

libgavia.a: contstream.o
libgavia.a: files...o
libgavia.a: taxon.o
libgavia.a: taxa.o
libgavia.a: date.o
libgavia.a: coordinate.o
libgavia.a: excursion.o
libgavia.a: excursion_check.o
libgavia.a: excursion_put.o
libgavia.a: indent.o
libgavia.a: regex.o
libgavia.a: filetest.o
libgavia.a: editor.o
libgavia.a: md5.o
libgavia.a: md5pp.o
libgavia.a: utf8.o
libgavia.a: version.o
	$(AR) -r $@ $^

groblad_cat: groblad_cat.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

groblad_grep: groblad_grep.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

groblad: groblad.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

groblad_report: groblad_report.o libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgavia

CFLAGS=-W -Wall -pedantic -ansi -g -Os
CXXFLAGS=-W -Wall -pedantic -std=c++11 -g -Os

.PHONY: check checkv
check: test/test
	./test/test
checkv: test/test
	valgrind -q ./test/test -v

test/libtest.a: test/test_taxa.o
test/libtest.a: test/test_coord.o
test/libtest.a: test/test_date.o
test/libtest.a: test/test_indent.o
test/libtest.a: test/test_cont.o
test/libtest.a: test/test_filetest.o
test/libtest.a: test/test_utf8.o
	$(AR) -r $@ $^

test/test_%.o: CPPFLAGS+=-I.

test/test: test/test.o test/libtest.a libgavia.a
	$(CXX) $(CXXFLAGS) -o $@ test/test.o -Ltest/ -ltest -L. -lgavia

test/test.cc: test/libtest.a
	orchis -o $@ $^

species_raw: kärlvl
species_raw: dyntaxa/Tracheophyta
	./kärlvl dyntaxa/Tracheophyta | unexpand -a >$@

.PHONY: install
install: all
	install -m555 groblad{,_cat,_grep,_report} $(INSTALLBASE)/bin/
	install -m555 build/groblad_fv.py $(INSTALLBASE)/bin/groblad_fv
	install -m644 build/*.1 $(INSTALLBASE)/man/man1/
	install -m644 build/*.5 $(INSTALLBASE)/man/man5/
	install -d $(INSTALLBASE)/lib/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad
	install -m644 linnaeus.nrm.se $(INSTALLBASE)/lib/groblad

$(INSTALLBASE)/lib/groblad:
	install -d $@

build/%: %
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@

.PHONY: tags
tags: TAGS
TAGS:
	etags *.{c,h,cc}

.PHONY: clean
clean:
	$(RM) groblad
	$(RM) groblad_cat
	$(RM) groblad_grep
	$(RM) groblad_report
	$(RM) build/groblad_fv.py
	$(RM) build/*.[15]
	$(RM) *.o lib*.a
	$(RM) test/test test/test.cc test/*.o test/lib*.a
	$(RM) *.pyc
	$(RM) version.c
	$(RM) species_raw
	$(RM) -r dep

love:
	@echo "not war?"

# DO NOT DELETE

$(shell mkdir -p dep/test)
DEPFLAGS=-MT $@ -MMD -MP -MF dep/$*.Td
COMPILE.cc=$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o: %.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	mv dep/$*.{Td,d}

dep/%.d: ;
dep/test/%.d: ;
-include dep/*.d
-include dep/test/*.d
