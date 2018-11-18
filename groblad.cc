/*
 * Copyright (c) 2013, 2014, 2018 Jörgen Grahn
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
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "taxa.h"
#include "files...h"
#include "excursion.h"
#include "lineparse.h"
#include "editor.h"
#include "filetest.h"
#include "md5pp.h"


extern "C" {
    const char* groblad_name();
    const char* groblad_version();
    const char* groblad_prefix();
}


namespace {

    std::string getenv(const char* name)
    {
	const char* p = std::getenv(name);
	return p? p: "";
    }

    /**
     * foo -> $HOME/foo
     */
    std::string tilde(const std::string& tail)
    {
	return getenv("HOME") + "/" + tail;
    }


    /**
     * Invent a temporary file name "/tmp/groblad.pid.gavia."
     * I'd rather not do this by hand, but the numerous library
     * functions are, as always, either deprecated or unsuitable.
     */
    std::string temp_name()
    {
	char buf[50];
	std::snprintf(buf, sizeof buf,
		      "/tmp/groblad.%x.gavia",
		      unsigned(getpid()));
	return buf;
    }


    std::string today()
    {
	char buf[4+3+3+1];
	const time_t t = time(0);
	struct tm tm;
	localtime_r(&t, &tm);
	std::strftime(buf, sizeof buf,
		      "%Y-%m-%d", &tm);
	return buf;
    }


    /**
     * Simply the MD5 digest of text file 'f'.  An unreadable file is
     * indistinguishable from an empty one.
     */
    md5::Digest md5sum(const std::string& f)
    {
	std::ifstream is(f);
	std::string s;
	md5::Ctx ctx;
	while(std::getline(is, s)) {
	    ctx.update(s);
	}
	return ctx.digest();
    }


    /**
     * Delete a file when we go out of scope.
     */
    struct Guard {
	explicit Guard(const std::string& f) : f(f) {}
	~Guard() {
	    if(f.empty()) return;
	    (void)unlink(f.c_str());
	}
	void unguard() { f.clear(); }
    private:
	std::string f;
    };


    /**
     * True if s matches /^\s*date\s*:\s*$/.
     */
    bool is_date_header(const std::string& s)
    {
	const char dateh [] = "date";

	const char* a = s.c_str();
	const char* const b = a + s.size();
	a = Parse::ws(a, b);
	if(a==b || *a != dateh[0]) return false;
	const char* c = a;
	while(c!=b && *c!=':') c++;
	if(c==b || Parse::ws(c+1, b) != b) return false;

	/* ok, everything is fine except we don't know
	 * that [a, c[ matches place\s*.
	 */
	c = Parse::trimr(a, c);
	if(c-a != sizeof dateh - 1) return false;
	return std::equal(a, c, dateh);
    }


    /**
     * Copy a template file from 'src' to 'dst', while filling in
     * today's date.
     */
    bool prepare(const std::string& src,
		 const std::string& dest)
    {
	const std::string& date = today();
	std::ifstream is(src);
	std::ofstream os(dest);
	if(!is || !os) return false;
	std::string s;
	while(std::getline(is, s)) {
	    if(is_date_header(s)) {
		os << s << date << '\n';
	    }
	    else {
		os << s << '\n';
	    }
	}
	/* XXX is fail() the right test? */
	return is.fail() + os.fail();
    }


    std::vector<Excursion> read(std::ostream& cerr,
				Taxa& taxa,
				const std::string& file)
    {
	Files in(&file, &file+1);
	std::vector<Excursion> acc;
	const Excursion nil;

	Excursion ex;
	while(get(in, cerr, taxa, ex)) {
	    acc.push_back(nil);
	    acc.back().swap(ex);
	}
	return acc;
    }


    /**
     * Filter 'file' through get(..., Excursion&) and write it back.
     * This removes unseen taxa, cleans up indentation, and so on.
     */
    bool rewrite(std::ostream& cerr,
		 Taxa& taxa,
		 const std::string& file)
    {
	const std::vector<Excursion> book = read(cerr, taxa, file);
	std::ofstream os(file);

	for(const Excursion& ex : book) {
	    os << ex << '\n';
	}

	if(!os || os.fail()) {
	    cerr << "error: failed to rewrite "
		 << file << ": "
		 << std::strerror(errno) << '\n';
	    return false;
	}

	return true;
    }


    /**
     * Like rewrite(), but appends to 'dest' instead of writing back.
     */
    bool rewrite_to(std::ostream& cerr,
		    Taxa& taxa,
		    const std::string& src,
		    const std::string& dest)
    {
	const std::vector<Excursion> book = read(cerr, taxa, src);
	std::ofstream os(dest, std::ios_base::app);

	for(const Excursion& ex : book) {
	    os << '\n' << ex;
	}

	if(!os || os.fail()) {
	    cerr << "error: failed to write "
		 << dest << ": "
		 << std::strerror(errno) << '\n';
	    return false;
	}

	return true;
    }


    /**
     * The main work, after a decent attempt to weed out non-existing
     * templates and unwritable books.
     */
    int stellata(std::ostream& cerr,
		 std::ifstream& species,
		 const std::string& extemplate,
		 const std::string& book)
    {
	const std::string tmp0 = temp_name();

	if(!prepare(extemplate, tmp0)) {
	    cerr << "failed to generate temporary file \""
		 << extemplate << "\": exiting\n";
	    return 1;
	}

	Guard guard(tmp0);

	const md5::Digest before = md5sum(tmp0);

	cerr << "Invoking editor... " << std::flush;
	if(!editor(tmp0, book)) {
	    cerr << '\n'
		 << "Failed; aborting.\n";
	    return 1;
	}

	const md5::Digest after = md5sum(tmp0);
	if(before==after) {
	    cerr << '\n'
		 << "Aborted unmodified excursion.\n";
	    return 0;
	}

	Taxa taxa(species, cerr);
	species.close();
	if(!rewrite(cerr, taxa, tmp0)) {
	    cerr << '\n'
		 << "Error: some problem rewriting the excursion; aborting.\n";
	    return 1;
	}

	cerr << "again...\n";
	if(!editor(tmp0, book)) {
	    cerr << "Failed; aborting.\n"
		 << "If you want your text back, look for " << tmp0 << ".\n";
	    guard.unguard();
	    return 1;
	}

	if(!rewrite_to(cerr, taxa, tmp0, book)) {
	    cerr << "Error: failed to add to " << book << ".\n"
		 << "If you want your text back, look for " << tmp0 << ".\n";
	    guard.unguard();
	    return 1;
	}

	return 0;
    }
}


int main(int argc, char ** argv)
{
    using std::string;

    const string prog = argv[0];
    const string usage = string("usage: ")
	+ prog + " [-f template] [-s species] file\n"
	"       "
	+ prog + " --version";
    const char optstring[] = "f:s:";
    const struct option long_options[] = {
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'H'},
	{0, 0, 0, 0}
    };

    std::cin.sync_with_stdio(false);
    std::cout.sync_with_stdio(false);

    std::string species_file = Taxa::species_file();
    string extemplate;

    int ch;
    while((ch = getopt_long(argc, argv,
			    optstring,
			    &long_options[0], 0)) != -1) {
	switch(ch) {
	case 'f':
	    extemplate = optarg;
	    break;
	case 's':
	    species_file = optarg;
	    break;
	case 'V':
	    std::cout << prog << ", part of "
		      << groblad_name() << ' ' << groblad_version() << "\n"
		      << "Copyright (c) 2004 - 2013, 2018 Jörgen Grahn\n";
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

    if(optind+1!=argc) {
	std::cerr << usage << '\n';
	return 1;
    }

    const std::string book = argv[optind];
    if(!filetest::writeable(book)) {
	std::cerr << prog << ": error: "
		  << book << ": "
		  << std::strerror(errno) << '\n';
	return 1;
    }

    if(extemplate.empty()) {
	extemplate = tilde(".flora");
	if(!filetest::readable(extemplate)) {
	    extemplate = groblad_prefix();
	    extemplate += "/lib/groblad/default";
	}
    }

    std::ifstream species(species_file);
    if(!species) {
        std::cerr << "error: cannot open '" << species_file
                  << "' for reading: " << std::strerror(errno) << '\n';
        return 1;
    }

    return stellata(std::cerr, species, extemplate, book);
}
