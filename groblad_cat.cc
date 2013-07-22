/*
 * Copyright (c) 1999--2001, 2013 Jörgen Grahn
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
#include <stdio.h>
#include <getopt.h>

#include "files...h"
#include "taxa.h"
#include "excursion.hh"
#include "mbox.h"


extern "C" {
    const char* gavia_name();
    const char* gavia_version();
    const char* gavia_prefix();
}


namespace {

    /**
     * Pipe through gavia_gab2roff etc.
     */
    FILE* popen(char kind)
    {
	const char* fmt;
	switch(kind) {
	case 't':
	default:
	    return 0;
	case 'm':
	    fmt = "mbox"; break;
	case 'h':
	    fmt = "html"; break;
	case 'r':
	    fmt = "roff"; break;
	}

	std::string cmd = gavia_prefix();
	cmd += "/bin/gavia_gab2";
	cmd += fmt;

	return ::popen(cmd.c_str(), "w");
    }
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [-gthmr] [-cx] file ...\n"
	"       "
	+ prog + " --check file ...\n"
	"       "
	+ prog + " --version";
    const char optstring[] = "gthmrcx";
    const struct option long_options[] = {
	{"check", 0, 0, 'C'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    bool sort_spp = false;
    char outfmt = 'g';

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 'g':
	case 'h':
	case 'm':
	case 'r':
	    outfmt = ch;
	    break;
	case 't':
	    outfmt = 'g';
	    break;
	case 'c':
	    sort_spp = false;
	    break;
	case 'x':
	    sort_spp = true;
	    break;
	case 'C':
	    outfmt = '-';
	    break;
	case 'V':
	    std::cout << prog << ", part of "
		      << gavia_name() << ' ' << gavia_version() << "\n"
		      << "Copyright (c) 1999 - 2013 Jörgen Grahn\n";
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

    if(outfmt=='g' || outfmt=='m') {
	Excursion ex;
	unsigned n = 0;
	while(get(files, std::cerr, taxa, ex)) {
	    if(n++) std::cout << '\n';

	    if(outfmt == 'm') {
		std::cout << MboxHeader(ex) << '\n';
	    }

	    ex.put(std::cout, sort_spp);
	}
    }
    else if(outfmt=='-') {
	Excursion ex;
	while(get(files, std::cerr, taxa, ex)) {
	    ;
	}
    }
    else {
	FILE* const f = popen(outfmt);
	if(!f) {
	    std::cerr << prog
		      << ": error: failed to open output formatter for '"
		      << outfmt << "': exiting\n";
	    return 1;
	}

	Excursion ex;
	while(get(files, std::cerr, taxa, ex)) {
	    if(!ex.put(f, sort_spp)) break;
	}

	if(pclose(f)==-1) {
	    std::cerr << prog
		      << ": error: output formatter failed\n";
	    return 1;
	}
    }

    return 0;
}
