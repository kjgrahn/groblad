/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "taxa.h"
#include "lineparse.h"

#include <iostream>
#include <algorithm>
#include <cassert>


using Parse::ws;
using Parse::trimr;
using Parse::find;


Taxa::Taxa(std::istream& is, std::ostream&)
{
    std::string s;
    unsigned n = 1;

    while(std::getline(is, s)) {
	const char* a = s.c_str();
	const char* b = a + s.size();

	a = ws(a, b);
	b = std::find(a, b, '#');
	b = trimr(a, b);
	if(a==b) continue;

	if(*a=='=' && !v.empty()) {
	    Taxon& sp = v.back();
	    a = ws(a+1, b);
	    const std::string alias(a, b-a);
	    sp.add(alias);
	    m[alias] = sp.id;
	}
	else {
	    const char* d = std::find(a, b, '(');
	    if(d==b) {
		const TaxonId id(n++);
		const std::string name(a, b-a);
		v.push_back(Taxon(id, name));
		m[name] = id;
	    }
	    else {
		const char* c = trimr(a, d);
		d++;
		const char* e = ::find(d, b, '(', ')');
		const TaxonId id(n++);
		const std::string name(a, c-a);
		const std::string latin(d, e-d);
		v.push_back(Taxon(id, name, latin));
		m[name] = id;
		m[latin] = id;
	    }
	}
    }
}


/**
 * Insert and assign an id to a new Taxon simply called 'name'.
 * Returns the nil id if find(name) already exists.
 */
TaxonId Taxa::insert(const std::string& name)
{
    TaxonId id;
    if(!find(name)) {
	id = TaxonId(v.size() + 1);
	v.push_back(Taxon(id, name));
	m[name] = id;
    }
    return id;
}


/**
 * Find a taxon by name (or scientific name, or alias)
 * and return its id.  Returns the nil id if not found.
 */
TaxonId Taxa::find(const std::string& name) const
{
    TaxonId id;
    Map::const_iterator i = m.find(name);
    if(i!=m.end()) {
	id = i->second;
    }
    return id;
}


/**
 * Return the taxon 'id', which must exist in this taxa list.
 */
const Taxon& Taxa::operator[] (TaxonId id) const
{
    const unsigned n = id.val - 1;
    const Taxon& sp = v.at(n);
    assert(sp.id==id);
    return sp;
}


/**
 * Return an ordered list of the taxa matching 're', in any of its
 * names.
 */
std::vector<TaxonId> Taxa::match(const Regex& re) const
{
    std::vector<TaxonId> acc;
    for(std::vector<Taxon>::const_iterator i = v.begin();
	i != v.end();
	i++) {
	if(i->match(re)) acc.push_back(i->id);
    }
    return acc;
}


extern "C" const char* groblad_prefix();

/**
 * The path to the file containing the default list of taxa.  Quite
 * unrelated to the rest of Taxa, which don't use it -- taxa are read
 * from a std::istream.
 */
std::string Taxa::species_file()
{
    std::string s = groblad_prefix();
    s += "/lib/gavia/species";
    return s;
}
