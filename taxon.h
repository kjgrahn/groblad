/* -*- c++ -*-
 *
 * Copyright (c) 1999, 2013, 2020 Jörgen Grahn
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
#ifndef GAVIA_TAXON_H
#define GAVIA_TAXON_H

#include <string>
#include <vector>
#include <iosfwd>


/**
 * A short reference to a species (taxon or name, really) in a Taxa
 * list.
 */
struct TaxonId {
    TaxonId() : val(0) {}
    explicit TaxonId(unsigned val) : val(val) {}
    unsigned short val;

    operator const void*() const { return val? this: 0; }
    bool operator< (const TaxonId& other) const { return val < other.val; }
    bool operator== (const TaxonId& other) const { return val == other.val; }
    bool operator!= (const TaxonId& other) const { return !(*this==other); }
};


class Regex;

/**
 * A species, or taxon in general.
 *
 * There's always a primary name, which usually is the common name.
 * The scientific (latin) name is optional, but will become the primary
 * name if there is no common name.  So the cases are:
 *
 * - gullviva
 * - gullviva, Primula veris
 * - Primula veris
 *
 * Then there are also optional aliases: other names applied to the
 * same taxon.  These are usually common names, but may also be
 * scientific names.
 *
 * Names are classified as genera, and other names (species, hybrids,
 * subspecies and so on). This is a bit vague, but intended to support
 * duplicate detection in common cases (it's normal to note two
 * "Alchemilla sp" in one field list, but not two "ormrot").
 */
class Taxon {
public:
    Taxon(TaxonId id, bool genus,
	  const std::string& name)
	: id(id),
	  genus(genus),
	  name(name)
    {}
    Taxon(TaxonId id, bool genus,
	  const std::string& name,
	  const std::string& latin)
	: id(id),
	  genus(genus),
	  name(name),
	  latin(latin)
    {}
    void add(const std::string& name) { alias.push_back(name); }

    bool operator< (const Taxon& other) const { return id < other.id; }
    std::ostream& put(std::ostream& os) const;

    bool match(const Regex& re) const;

    const TaxonId id;
    const bool genus;
    const std::string name;
    const std::string latin;
    std::vector<std::string> alias;
};


inline std::ostream& operator<< (std::ostream& os, const Taxon& s)
{
    return s.put(os);
}

#endif
