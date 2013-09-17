/*
 * Copyright (c) 2013 Jörgen Grahn
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
#include <stdio.h>
#include <getopt.h>

#include "files...h"
#include "taxa.h"
#include "excursion.h"


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

    void troff(std::ostream& os,
	       const Book& book,
	       const Taxa& spp)
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
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [--ms] file ...\n"
	"       "
	+ prog + " --sundh file ...\n"
	"       "
	+ prog + " --svalan [-s] [--me name] ... file ...\n"
	"       "
	+ prog + " --version\n"
	"       "
	+ prog + " --help";
    const char optstring[] = "s";
    const struct option long_options[] = {
	{"ms", 0, 0, 'M'},
	{"sundh", 0, 0, 'Z'},
	{"svalan", 0, 0, 'S'},
	{"me", 1, 0, 'E'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    bool filter_unknown = false;
    bool generate_troff = true;
    bool generate_sundh = false;
    std::vector<string> me;

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 's': filter_unknown = true; break;
	case 'M': generate_troff = true; break;
	case 'Z': generate_troff = false; generate_sundh = true; break;
	case 'S': generate_troff = false; generate_sundh = false; break;
	case 'E': me.push_back(optarg); break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2004 - 2013 Jörgen Grahn\n";
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

    if(filter_unknown || generate_sundh) {
	std::cerr << "Unsupported mode (so far); use --svalan. Sorry!\n";
	return 1;
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

    return 0;
}
