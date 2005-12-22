# $Id: Makefile,v 1.3 2005-12-22 22:42:33 grahn Exp $

SHELL = /bin/sh

INSTALLBASE = /usr/local

all: _groblad.pl _groblad_report.py
all: _groblad.1 _groblad.5 _groblad_report.1
all: default species

.PHONY: install
install: all
	install -m555 _groblad.pl $(INSTALLBASE)/bin/groblad
	install -m555 _groblad_report.py $(INSTALLBASE)/bin/groblad_report
	install -d $(INSTALLBASE)/lib/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad
	install -m644 _groblad.1 $(INSTALLBASE)/man/man1/groblad.1
	install -m644 _groblad_report.1 $(INSTALLBASE)/man/man1/groblad_report.1
	install -m644 _groblad.5 $(INSTALLBASE)/man/man5/groblad.5

$(INSTALLBASE)/lib/groblad:
	install -d $@

_groblad.pl: groblad.pl
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@
_groblad_report.py: groblad_report.py
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@
_groblad.1: groblad.1
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@
_groblad.5: groblad.5
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@
_groblad_report.1: groblad_report.1
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@

clean:
	$(RM) _groblad.pl _groblad_report.py _groblad.1 _groblad.5 _groblad_report.1

love:
	@echo "not war?"
