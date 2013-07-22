/*----------------------------------------------------------------------------
 *
 * Copyright (c) 1999, 2000, 2008, 2013 J�rgen Grahn
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
#ifndef EXCURSION_HH
#define EXCURSION_HH

#include "taxon.h"
#include "date.h"

#include <string>
#include <vector>
#include <iosfwd>
#include <cstdio>


class Taxa;
class Files;

/**
 * An excursion, as it appears in a gavia(5) file but as an internal
 * representation.  Still, does not destroy information found in
 * a well-formed excursion, e.g. the ordering of species and unknown
 * header fields.
 *
 * Does not destroy /much/ anyway. What's not preserved is:
 * - unseen (unmarked and no numbers or comment given) species
 * - the marker used for the seen ones
 * - spacing and alignment (but line breaks/continuations are preserved,
 *   as just an '\n')
 * - # comments
 */
class Excursion
{
public:
    struct Header {
	Header(const std::string& name, const std::string& value)
	    : name(name), value(value) {}
	std::string name;
	std::string value;
    };
    struct Sighting {
	Sighting(TaxonId sp,
		 const std::string& name,
		 const std::string& number,
		 const std::string& comment)
	    : sp(sp),
	      name(name),
	      number(number),
	      comment(comment)
	{}
	TaxonId sp;
	std::string name;
	std::string number;
	std::string comment;
	bool operator< (const Sighting& other) const {
	    return sp < other.sp;
	}
    };

    void swap(Excursion& other);

    bool add_header(const char* a, size_t alen,
		    const char* b, size_t blen);
    bool add_header_cont(const char* a, size_t alen);

    bool add_sighting(Taxa& spp,
		      const char* a, size_t alen,
		      const char* b, size_t blen,
		      const char* c, size_t clen);
    bool add_sighting_cont(const char* a, size_t alen);
    bool finalize();

    bool operator< (const Excursion& other) const { return date < other.date; }
    bool has_one(const std::vector<TaxonId>& taxa) const;

    std::ostream& put(std::ostream& os,
		      bool sort = false, size_t indent = 16) const;
    bool put(FILE* f, bool sort = false, size_t indent = 16) const;

    typedef std::vector<Header> Headers;
    typedef std::vector<Sighting> Sightings;
    Headers::const_iterator hbegin() const { return headers.begin(); }
    Headers::const_iterator hend() const { return headers.end(); }
    Sightings::const_iterator sbegin() const { return sightings.begin(); }
    Sightings::const_iterator send() const { return sightings.end(); }

    const std::string& find_header(const char* name) const;

    Date date;
    std::string place;

private:
    Headers headers;
    Sightings sightings;
};


bool get(Files& is, std::ostream& errstream,
	 Taxa& spp, Excursion& excursion);

#endif
