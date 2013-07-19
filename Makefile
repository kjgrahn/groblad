# $Id: Makefile,v 1.10 2011-07-16 13:22:38 grahn Exp $

SHELL = /bin/sh

INSTALLBASE = /usr/local

all: _groblad.pl _groblad_report.py
all: _groblad.1 _groblad.5 _groblad_report.1 groblad_grep.1
all: groblad_grep.py
all: groblad_fv.py groblad_fv.1
all: default species

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

species_raw: kärlvl
species_raw: dyntaxa/Tracheophyta
	./kärlvl dyntaxa/Tracheophyta | unexpand -a >$@

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

clean:
	$(RM) _groblad.pl _groblad_report.py
	$(RM) _groblad.1 _groblad.5 _groblad_report.1
	$(RM) species_raw
	$(RM) falat
	$(RM) karlsson/falat*.html

love:
	@echo "not war?"
