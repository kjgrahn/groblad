/*
 * Copyright (c) 1999--2001, 2013 J�rgen Grahn
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


int main(int argc, char ** argv)
{
    const std::string prog = argv[0];
    const std::string usage = std::string("usage: ")
	+ prog + " [-cx] [-s species] file ...\n"
	"       "
	+ prog + " [-s species] --check file ...\n"
	"       "
	+ prog + " [-s species] --taxa\n"
	"       "
	+ prog + " --version";
    const char optstring[] = "gcxs:";
    const struct option long_options[] = {
	{"check", 0, 0, 'C'},
	{"taxa", 0, 0, 'T'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    std::string species_file = Taxa::species_file();
    bool just_list_taxa = false;
    bool sort_spp = false;
    char outfmt = 'g';

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 's':
	    species_file = optarg;
	    break;
	case 'c':
	    sort_spp = false;
	    break;
	case 'x':
	    sort_spp = true;
	    break;
	case 'T':
	    just_list_taxa = true;
	    break;
	case 'C':
	    outfmt = '-';
	    break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2004 - 2013, 2020 J�rgen Grahn\n";
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

    std::ifstream species(species_file);
    if(!species) {
        std::cerr << "error: cannot open '" << species_file
                  << "' for reading: " << std::strerror(errno) << '\n';
        return 1;
    }
    Taxa taxa(species, std::cerr);
    species.close();

    if(just_list_taxa) {
	taxa.put(std::cout);
    }
    else if(outfmt=='g') {
	Excursion ex;
	unsigned n = 0;
	while(get(files, std::cerr, taxa, ex)) {
	    if(n++) std::cout << '\n';

	    ex.put(std::cout, sort_spp);
	}
    }
    else if(outfmt=='-') {
	Excursion ex;
	while(get(files, std::cout, taxa, ex)) {
	    ;
	}
    }

    return 0;
}
