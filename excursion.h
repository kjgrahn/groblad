/*----------------------------------------------------------------------------
 *
 * Copyright (c) 1999, 2000, 2008, 2013 Jörgen Grahn
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
#include <iostream>
#include <cstdio>


class Taxa;
class Files;

/**
 * A field list, or excursion, as it appears in a groblad(5) file but
 * as an internal representation.  Still, does not destroy information
 * found in a well-formed excursion, e.g. the ordering of taxa and
 * unknown header fields.
 *
 * Does not destroy /much/ anyway. What's not preserved is:
 * - unseen (unmarked and no numbers or comment given) species
 * - the marker used for the seen ones
 * - spacing and alignment (but line breaks/continuations are preserved,
 *   as just an '\n')
 * - # comments
 *
 * The terminology is off because it's copied from Gavia.  An
 * excursion there is a period of time perhaps visiting several
 * localities; a field list here covers one locality.
 *
 * Field list is also not quite right, since that refers to a paper
 * you bring into the field, rather than a final and permanent record.
 */
class FieldList
{
public:
    struct Header {
	Header(const std::string& name, const std::string& value)
	    : name(name), value(value) {}
	std::string name;
	std::string value;
	bool empty() const { return value.empty(); }
    };
    struct Sighting {
	Sighting(TaxonId sp,
		 const std::string& name,
		 const std::string& comment)
	    : sp(sp),
	      name(name),
	      comment(comment)
	{}
	TaxonId sp;
	std::string name;
	std::string comment;
	bool operator< (const Sighting& other) const {
	    return sp < other.sp;
	}
    };

    void swap(FieldList& other);

    bool add_header(const char* a, size_t alen,
		    const char* b, size_t blen);
    bool add_header_cont(const char* a, size_t alen);

    bool add_sighting(Taxa& spp,
		      const char* a, size_t alen,
		      const char* b, size_t blen);
    bool add_sighting_cont(const char* a, size_t alen);
    bool finalize();

    bool operator< (const FieldList& other) const { return date < other.date; }
    bool has_one(const std::vector<TaxonId>& taxa) const;
    bool contains(TaxonId taxon) const;

    std::ostream& put(std::ostream& os,
		      bool sort = false) const;
    bool put(FILE* f, bool sort = false) const;

    typedef std::vector<Header> Headers;
    typedef std::vector<Sighting> Sightings;
    Headers::const_iterator hbegin() const { return begin(headers); }
    Headers::const_iterator hend() const { return end(headers); }
    Sightings::const_iterator sbegin() const { return begin(sightings); }
    Sightings::const_iterator send() const { return end(sightings); }

    bool has_header(const std::string& name) const;
    const std::string& find_header(const char* name) const;

    Date date;
    std::string place;

private:
    Headers headers;
    Sightings sightings;
};

/* compatibility name */
using Excursion = FieldList;


void check_sightings(const Excursion& ex, Taxa& taxa,
		     const Files& is, std::ostream& err);
void check_last_header(const Excursion& ex,
		       const Files& is, std::ostream& err);
void check_last_sighting(const Excursion& ex,
			 const Files& is, std::ostream& err);

bool get(Files& is, std::ostream& errstream,
	 Taxa& spp, Excursion& excursion);

inline
std::ostream& operator<< (std::ostream& os, const Excursion& val)
{
    return val.put(os);
}

#endif
