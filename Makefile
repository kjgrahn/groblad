# $Id: Makefile,v 1.6 2010-03-22 22:26:50 grahn Exp $

SHELL = /bin/sh

INSTALLBASE = /usr/local

all: _groblad.pl _groblad_report.py
all: _groblad.1 _groblad.5 _groblad_report.1 groblad_grep.1
all: groblad_grep.py
all: default species

.PHONY: install
install: all
	install -m555 _groblad.pl $(INSTALLBASE)/bin/groblad
	install -m555 _groblad_report.py $(INSTALLBASE)/bin/groblad_report
	install -m555 groblad_grep.py $(INSTALLBASE)/bin/groblad_grep
	install -d $(INSTALLBASE)/lib/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad
	install -m644 _groblad.1 $(INSTALLBASE)/man/man1/groblad.1
	install -m644 _groblad_report.1 $(INSTALLBASE)/man/man1/groblad_report.1
	install -m644 groblad_grep.1 $(INSTALLBASE)/man/man1/groblad_grep.1
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
	$(RM) _groblad.pl _groblad_report.py
	$(RM) _groblad.1 _groblad.5 _groblad_report.1

love:
	@echo "not war?"
