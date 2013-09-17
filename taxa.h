/* -*- c++ -*-
 *
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
#ifndef GAVIA_TAXA_H
#define GAVIA_TAXA_H

#include "taxon.h"

#include <vector>
#include <tr1/unordered_map>
#include <iosfwd>


class Regex;

/**
 * A "species list": a list of taxa in roughly taxonomical order with
 * - a swedish name (in mostly lowercase)
 * - a scientific name (optional)
 * - a number of synonyms (optional)
 * - an implicit order within the list
 * - an implicit numerical ID
 *
 * Genera are also available, as e.g. "Primula sp".
 *
 * Such a list is typically initialized from a text file, but there
 * are provisions for adding to it.
 */
class Taxa {
public:
    Taxa(std::istream& is, std::ostream& err);

    TaxonId insert(const std::string& name);

    TaxonId find(const std::string& name) const;
    const Taxon& operator[] (TaxonId id) const;

    std::vector<TaxonId> match(const Regex& re) const;
    std::ostream& put(std::ostream& os) const;

    typedef std::vector<Taxon>::const_iterator const_iterator;
    const_iterator begin() const { return v.begin(); }
    const_iterator end() const { return v.end(); }

    static std::string species_file();

private:
    std::vector<Taxon> v;
    typedef std::tr1::unordered_map<std::string, TaxonId> Map;
    Map m;

    void map(const std::string& name, TaxonId id, std::ostream& err);
};

#endif
