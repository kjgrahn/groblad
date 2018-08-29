/*
 * Copyright (c) 2018 Jörgen Grahn
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
#include <iostream>
#include <cstring>
#include <algorithm>
#include <getopt.h>

#include "files...h"
#include "taxa.h"
#include "excursion.h"
#include "comments.h"
#include "indent.h"
#include "lineparse.h"


extern "C" {
    const char* groblad_name();
    const char* groblad_version();
}


namespace {

    /**
     * All taxon names, aliases and all, as a set.
     */
    std::set<std::string> names(const Taxa& taxa)
    {
	const auto v = taxa.names();
	return {begin(v), end(v)};
    }

    /**
     * The n:th substring in 'pc', with whitespace trimmed.
     */
    std::string trimmed(const std::vector<const char*>& pc,
			size_t n)
    {
	auto a = pc[n];
	auto b = pc[n+1];
	a = Parse::ws(a, b);
	b = Parse::trimr(a, b);
	return {a, b};
    }

    struct ParsedComment {

	ParsedComment(const std::set<std::string>& names,
		      const std::string& s);

	std::string pre;

	struct Sighting {
	    std::string name;
	    std::string comment;
	};
	std::vector<Sighting> sightings;
    };

    ParsedComment::ParsedComment(const std::set<std::string>& names,
				 const std::string& s)
    {
	const auto pc = comment::parse(names, s);
	pre = trimmed(pc, 0);
	const size_t spp = pc.size()/2 - 1;
	for(unsigned i=0; i<spp; i++) {
	    sightings.push_back({trimmed(pc, 2*i + 1),
				 trimmed(pc, 2*i + 2)});
	}
    }

    /**
     * The maximum width of iter->name.
     */
    template<class It>
    size_t max_name(Indent& indent, It a, It b)
    {
	size_t n = 0;
	while(a!=b) {
	    size_t m = indent.measure(a->name);
	    if(m>n) n=m;
	    a++;
	}
	return n;
    }

    void print(std::ostream& os, Indent& indent,
	       const Excursion& ex,
	       const ParsedComment& pc)
    {
	const bool synth_coordinate = !ex.has_header("coordinate");

	os << "{\n";
	{
	    const size_t n = std::max(std::strlen("coordinate"),
				      max_name(indent, ex.hbegin(),
					       ex.hend())) + 1;
	    const Date& d = ex.date;
	    /* Printing a Header with the colon in
	     * column 'n', as
	     *
	     * name     : text text text
	     *            text text ...
	     */
	    auto print = [&os, &indent, n,
			  &d, &synth_coordinate] (const Excursion::Header& h) {
			     if(h.name == "comments") return;
			     indent.ljust(os, h.name, n) << ": ";
			     if(h.name != "date") {
				 indent.andjust(os, h.value, n+2) << '\n';
				 if(h.name == "place" && synth_coordinate) {
				     indent.ljust(os, "coordinate", n) << ": \n";
				 }
			     }
			     else {
				 os << d << '\n';
			     }
			 };

	    std::for_each(ex.hbegin(), ex.hend(), print);

	    if(!pc.pre.empty()) {
		indent.ljust(os, "comments", n) << ": ";
		indent.andjust(os, pc.pre, n+2) << '\n';
	    }
	}

	os << "}{\n";
	{
	    const size_t indentation = 16;
	    size_t m = std::max(indentation,
				max_name(indent, begin(pc.sightings),
					 end(pc.sightings)) + 1);
	    /**
	     * Printing a Sighting as
	     *
	     * <    m    >
	     * taxon      :#: text text text
	     *                text text ...
	     */
	    auto print = [&os, &indent, m](const ParsedComment::Sighting& s) {
			     indent.ljust(os, s.name, m) << ":#: ";
			     indent.andjust(os, s.comment, m+4) << '\n';
			 };

	    std::for_each(begin(pc.sightings), end(pc.sightings), print);
	}
	os << "}\n";
    }
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [-s species] file ...\n"
	"       "
	+ prog + " --version";
    const char optstring[] = "s:";
    const struct option long_options[] = {
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    std::string taxa_file;

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 's':
	    taxa_file = optarg;
	    break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2018 Jörgen Grahn\n";
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

    if(taxa_file.empty()) {
	std::cerr << "error: required argument missing\n"
		  << usage << '\n';
	return 1;
    }

    std::ifstream species(taxa_file, std::ios_base::in);
    if(!species) {
	std::cerr << "error: cannot open '" << taxa_file
		  << "' for reading: " << std::strerror(errno) << '\n';
	return 1;
    }
    Taxa taxa(species, std::cerr);
    species.close();
    const auto taxa_set = names(taxa);

    species.open(Taxa::species_file(), std::ios_base::in);
    Taxa gtaxa(species, std::cerr);
    species.close();

    Files files(argv+optind, argv+argc);

    Indent indent;
    Excursion ex;
    unsigned n = 0;
    while(get(files, std::cerr, gtaxa, ex)) {
	const auto& comments = ex.find_header("comments");
	if(comments.empty()) continue;

	if(n++) std::cout << '\n';

	print(std::cout, indent,
	      ex, ParsedComment(taxa_set, comments));
    }

    return 0;
}
