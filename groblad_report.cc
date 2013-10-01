/*
 * Copyright (c) 2013 J�rgen Grahn
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>

#include "files...h"
#include "taxa.h"
#include "excursion.h"
#include "coordinate.h"


extern "C" {
    const char* groblad_name();
    const char* groblad_version();
    const char* groblad_prefix();
}


namespace {

    /* Stolen from C++11.
     */
    template<typename InputIterator, typename OutputIterator,
	     typename Predicate>
    OutputIterator
    copy_if(InputIterator first, InputIterator last,
	    OutputIterator result, Predicate pred)
    {
	for (; first != last; ++first)
	    if (pred(*first))
	    {
		*result = *first;
		++result;
	    }
	return result;
    }

    struct contains {
	explicit contains(const Taxon& sp) : id(sp.id) {}
	explicit contains(const TaxonId& id) : id(id) {}
	const TaxonId id;
	template<class T>
	bool operator() (const T& t) const { return t.contains(id); }
    };

    typedef std::list<Excursion> Book;

    Book filter(const Book& src, const Taxon& sp)
    {
	Book acc;
	copy_if(src.begin(), src.end(),
		std::back_insert_iterator<Book>(acc),
		contains(sp));
	return acc;
    }

    void troff(std::ostream& os,
	       const Excursion& ex)
    {
	os << ex.find_header("place") << '\n'
	   << "\\s-2(" << ex.find_header("coordinate") << ")\\s0.\n"
	   << ex.find_header("observers")
	   << "\\~" << ex.find_header("date") << ".\n";
    }

    void troff(std::ostream& os, const Book& book, const Taxa& spp)
    {
	for(Taxa::const_iterator i=spp.begin(); i!=spp.end(); i++) {
	    const Taxon& sp = *i;
	    const Book b = filter(book, sp);
	    if(b.empty()) continue;

	    os << ".\n"
	       << ".XP\n"
	       << ".B \"" << sp.name << "\"\n";
	    if(!sp.latin.empty()) {
		os << ".I \"" << sp.latin << "\" :\n";
	    }

	    for(Book::const_iterator i=b.begin(); i!=b.end(); i++) {
		if(i!=b.begin()) os << "\\(em\n";
		troff(os, *i);
	    }
	}
    }

    std::string join(const std::string& s)
    {
	std::string t = s;
	std::replace(t.begin(), t.end(), '\n', ' ');
	return t;
    }

    struct exrow {
	exrow(std::ostream& os, const Taxa& spp, const Excursion& ex);
	void operator() (const Excursion::Sighting&) const;
	std::ostream& os;
	const Taxa& spp;
	const Excursion& ex;
	const std::string place;
	const std::string& date;
	const std::string& coordinate;
	const Coordinate coord;
    };

    exrow::exrow(std::ostream& os, const Taxa& spp, const Excursion& ex)
	: os(os),
	  spp(spp),
	  ex(ex),
	  place(join(ex.find_header("place"))),
	  date(ex.find_header("date")),
	  coordinate(ex.find_header("coordinate")),
	  coord(&coordinate[0], &coordinate[0] + coordinate.size())
    {}

    void exrow::operator() (const Excursion::Sighting& s) const
    {
	const Taxon sp = spp[s.sp];
	if(!sp.latin.empty()) {
	    os << sp.latin;
	}
	else {
	    os << sp.name;
	}

	os << "\t\t\t\t\t" << place;

	if(coord.valid()) {
	    os << '\t'
	       << coord.east << '\t'
	       << coord.north << '\t'
	       << coord.resolution;
	}
	else {
	    os << "\t\t\t";
	}

	os << '\t' << date
	   << '\t' << date
	   << "\t\t\t" << join(s.comment);

	os << '\n';
    }

    void tbl(std::ostream& os, const Book& book, const Taxa& spp)
    {
	os << ".TS H\n"
	   << "allbox;\n"
	   << "lrllllrrrlll llllllllllllllllllllllllllllll.\n";

	os << "artnamn"
	   << '\t' << "antal"
	   << '\t' << "enhet"
	   << '\t' << "�lder-stadium"
	   << '\t' << "k�n"
	   << '\t' << "lokalnamn"
	   << '\t' << "ost"
	   << '\t' << "nord"
	   << '\t' << "noggrannhet"
	   << '\t' << "startdatum"
	   << '\t' << "slutdatum"
	   << '\t' << "starttid"
	   << '\t' << "sluttid"
	   << '\t' << "publik kommentar"
	   << '\t' << "intressant notering"
	   << '\t' << "privat kommentar"
	   << '\t' << "ej �terfunnen"
	   << '\t' << "d�lj fyndet t.o.m."
	   << '\t' << "andrahand"
	   << '\t' << "os�ker artbest�mning"
	   << '\t' << "ospontan"
	   << '\t' << "biotop"
	   << '\t' << "biotop-beskrivning"
	   << '\t' << "art som substrat"
	   << '\t' << "art som substrat beskrivning"
	   << '\t' << "substrat"
	   << '\t' << "substrat-beskrivning"
	   << '\t' << "offentlig samling"
	   << '\t' << "privat samling"
	   << '\t' << "samlings-nummer"
	   << '\t' << "artbest�md av"
	   << '\t' << "best�mningsdatum"
	   << '\t' << "beskrivning artbest�mning"
	   << '\t' << "bekr�ftad av"
	   << '\t' << "med-observat�r"
	   << '\n'
	   << ".TH\n";

	for(Book::const_iterator i=book.begin(); i!=book.end(); i++) {
	    const Excursion& ex = *i;

	    std::for_each(ex.sbegin(), ex.send(), exrow(os, spp, ex));
	}
    }
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [--ms] file ...\n"
	"       "
	+ prog + " --svalan file ...\n"
	"       "
	+ prog + " --version\n"
	"       "
	+ prog + " --help";
    const char optstring[] = "";
    const struct option long_options[] = {
	{"ms", 0, 0, 'M'},
	{"svalan", 0, 0, 'S'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    bool generate_troff = true;

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 'M': generate_troff = true; break;
	case 'S': generate_troff = false; break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2004 - 2013 J�rgen Grahn\n";
	    return 0;
	    break;
	case 'H':
	    std::cout << usage << '\n';
	    return 0;
	    break;
	case ':':
	case '?':
	    std::cerr << usage << '\n';
	    return 1;
	    break;
	default:
	    break;
	}
    }

    Files files(argv+optind, argv+argc);
    std::ifstream species(Taxa::species_file().c_str());
    Taxa taxa(species, std::cerr);

    std::list<Excursion> book;
    const Excursion nil;
    Excursion ex;
    while(get(files, std::cerr, taxa, ex)) {
	book.push_back(nil);
	book.back().swap(ex);
    }

    if(generate_troff) {
	troff(std::cout, book, taxa);
    }
    else {
	tbl(std::cout, book, taxa);
    }

    return 0;
}