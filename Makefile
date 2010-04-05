# $Id: Makefile,v 1.7 2010-04-05 14:33:31 grahn Exp $

SHELL = /bin/sh

INSTALLBASE = /usr/local

all: _groblad.pl _groblad_report.py
all: _groblad.1 _groblad.5 _groblad_report.1 groblad_grep.1
all: groblad_grep.py
all: default species

falat%.html: falat%.htm
	perl -pe 's/ width=".+?"//g' <$< >$@

falat:\
falat1.html\
falat1b.html\
falat2.html\
falat2b.html\
falat3.html\
falat3b.html\
falat3c.html\
falat4.html\
falat5.html\
falat5b.html\
falat6.html\
falat7.html\
falat7b.html\
falat8.html\
falat9.html\
falat9b.html\
falat10.html\
falat11.html\
falat11b.html\
falat12.html\
falat13.html\
falat14.html\
falat14b.html\
falat14c.html\
falat15.html\
falat15b.html\
falat16.html\
falat17.html\
falat18.html\
falat18b.html
	( for f in $^ ; do w3m -cols 2000 -dump $$f ; done) >$@

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
	$(RM) falat
	$(RM) falat*.html

love:
	@echo "not war?"
