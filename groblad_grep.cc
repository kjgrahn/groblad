/*
 * Copyright (c) 1999--2001, 2013, 2018 Jörgen Grahn
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
#include <getopt.h>

#include "files...h"
#include "taxa.h"
#include "excursion.h"
#include "regex.h"


extern "C" {
    const char* groblad_name();
    const char* groblad_version();
}


namespace {

    /**
     * True if 'ex' matches 're' in some way, or if it contains
     * one of 'taxa'.
     */
    template<class Regex>
    bool matches(const Regex& re, const Excursion& ex,
		 const std::vector<TaxonId>& taxa)
    {
	if(ex.has_one(taxa)) return true;
	for(Excursion::Headers::const_iterator i = ex.hbegin();
	    i != ex.hend();
	    i++) {
	    if(re.match(i->value)) return true;
	}
	for(Excursion::Sightings::const_iterator i = ex.sbegin();
	    i != ex.send();
	    i++) {
	    if(re.match(i->name)) return true;
	    if(re.match(i->comment)) return true;
	}

	return false;
    }
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [-s species] [-v] pattern file ...\n"
	"       "
	+ prog + " --version";
    const char optstring[] = "vs:";
    const struct option long_options[] = {
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    std::string species_file = Taxa::species_file();
    bool invert = false;

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 'v':
	    invert = true;
	    break;
	case 's':
	    species_file = optarg;
	    break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2013, 2018 Jörgen Grahn\n";
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

    if(optind==argc) {
	std::cerr << usage << '\n';
	return 1;
    }

    const string rest = argv[optind++];
    const Regex re(rest);
    if(re.bad()) {
	std::cerr << prog << ": error in \""
		  << rest << "\": "
		  << re.error() << '\n';
	return 1;
    }

    Files files(argv+optind, argv+argc);

    std::ifstream species(species_file);
    if(!species) {
        std::cerr << "error: cannot open '" << species_file
                  << "' for reading: " << std::strerror(errno) << '\n';
        return 1;
    }
    Taxa taxa(species, std::cerr);
    species.close();
    const std::vector<TaxonId> matchtx = taxa.match(re);

    Excursion ex;
    unsigned n = 0;
    while(get(files, std::cerr, taxa, ex)) {

	if(invert ^ matches(re, ex, matchtx)) {
	    if(n++) std::cout << '\n';
	    std::cout << ex;
	}
    }

    return 0;
}
