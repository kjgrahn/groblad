# $Id: Makefile,v 1.1 2004-08-14 21:14:22 grahn Exp $

SHELL = /bin/sh

INSTALLBASE = /usr/local

all: _groblad.pl default species

install: _groblad.pl
install: $(INSTALLBASE)/lib/groblad
install: default species
	install -m555 _groblad.pl $(INSTALLBASE)/bin/groblad
	install -m644 species $(INSTALLBASE)/lib/groblad
	install -m644 default $(INSTALLBASE)/lib/groblad

$(INSTALLBASE)/lib/groblad:
	install -d $@

_groblad.pl: groblad.pl
	sed "s|INSTALLBASE|$(INSTALLBASE)|" <$< >$@

clean:
	$(RM) _groblad.pl

love:
	@echo "not war?"
