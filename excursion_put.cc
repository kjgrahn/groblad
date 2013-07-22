/* Copyright (c) 2013 J�rgen Grahn
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
#include "excursion.h"
#include "indent.h"

#include <iostream>
#include <algorithm>
#include <sstream>


namespace {

    /**
     * The maximum size() of iter->name.
     */
    template<class It>
    size_t max_name(It a, It b)
    {
	size_t n = 0;
	while(a!=b) {
	    size_t m = a->name.size();
	    if(m>n) n=m;
	    a++;
	}
	return n;
    }

    template<class It>
    size_t max_number(It a, It b)
    {
	size_t n = 0;
	while(a!=b) {
	    size_t m = a->number.size();
	    if(m>n) n=m;
	    a++;
	}
	return n;
    }

    /**
     * Printing a Header with the colon in
     * column 'n', as
     *
     * name     : text text text
     *            text text ...
     */
    struct PrintHeader {
	PrintHeader(std::ostream& os, size_t n)
	    : os(os),
	      n(n)
	{}
	void operator() (const Excursion::Header& val);
	std::ostream& os;
	const size_t n;
    };

    void PrintHeader::operator() (const Excursion::Header& val)
    {
	indent::ljust(os, val.name, n) << ": ";
	indent::andjust(os, val.value, n+2) << '\n';
    }

    /**
     * Printing a Sighting as
     *
     * <    m    >   < n >
     * taxon      :#:  nnn: text text text
     *                      text text ...
     */
    struct PrintSighting {
	PrintSighting(std::ostream& os, size_t m, size_t n)
	    : os(os),
	      m(m),
	      n(n)
	{}
	void operator() (const Excursion::Sighting& val);
	std::ostream& os;
	const size_t m;
	const size_t n;
    };

    void PrintSighting::operator() (const Excursion::Sighting& val)
    {
	indent::ljust(os, val.name, m) << ":#:";
	indent::rjust(os, val.number, n) << ": ";
	indent::andjust(os, val.comment, m+3+n+2) << '\n';
    }
}


/**
 * Write in 'gab' format to 'os'. Little translation is done: may
 * optionally sort the sightings according to taxonomic order, but
 * species names are printed as they were entered, empty headers are
 * printed and so on ... more effort is spent on proper indentation.
 *
 * Indents the text after the taxon name to at least column 'indent';
 * more if needed to avoid jaggedness.
 */
std::ostream& Excursion::put(std::ostream& os,
			     const bool sort,
			     const size_t indent) const
{
    size_t m, n;
    os << "{\n";
    n = max_name(headers.begin(), headers.end()) + 1;
    std::for_each(headers.begin(), headers.end(),
		  PrintHeader(os, n));

    Sightings sorted;
    if(sort) {
	sorted = sightings;
	std::stable_sort(sorted.begin(), sorted.end());
    }
    const Sightings& ss = sort ? sorted : sightings;

    os << "}{\n";
    m = std::max(indent, max_name(ss.begin(), ss.end()) + 1);
    n = max_number(ss.begin(), ss.end()) + 1;
    std::for_each(ss.begin(), ss.end(),
		  PrintSighting(os, m, n));

    return os << "}\n";
}


/**
 * Like put(ostream&, ...) but using stdio so it can be used with
 * popen(3).  Returns true if the writing worked.
 */
bool Excursion::put(FILE* f,
		    const bool sort,
		    const size_t indent) const
{
    std::ostringstream oss;
    put(oss, sort, indent);
    const std::string s = oss.str();
    size_t n = std::fwrite(s.data(), s.size(), 1, f);
    return n==1;
}