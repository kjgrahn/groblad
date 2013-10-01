/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "taxa.h"
#include "lineparse.h"

#include <tr1/unordered_set>
#include <iostream>
#include <algorithm>
#include <cassert>


namespace {

    /**
     * Just a helper class, for processing a genus name only once.
     */
    class Genera {
	std::tr1::unordered_set<std::string> s;
    public:
	bool seen(const char* a, const char* b);
    };

    bool Genera::seen(const char* a, const char* b)
    {
	const std::string name(a, b-a);
	return !s.insert(name).second;
    }
}


/**
 * Create the list from a text file in the format documented
 * in 'species':
 *
 *   common name
 *   common name  (scientific name)
 *   -            (scientific name)  for taxa with no swedish name
 *   = name                          a synonym for the previous name
 */
Taxa::Taxa(std::istream& is, std::ostream& err)
{
    Genera genera;
    std::string s;
    unsigned n = 1;

    while(std::getline(is, s)) {
	const char* a = s.c_str();
	const char* b = a + s.size();

	a = Parse::ws(a, b);
	b = std::find(a, b, '#');
	b = Parse::trimr(a, b);
	if(a==b) continue;

	if(*a=='=' && !v.empty()) {
	    Taxon& sp = v.back();
	    a = Parse::ws(a+1, b);
	    const std::string alias(a, b-a);
	    sp.add(alias);
	    map(alias, sp.id, err);
	}
	else {
	    const char* d = std::find(a, b, '(');
	    if(d==b) {
		const TaxonId id(n++);
		const std::string name(a, b-a);
		v.push_back(Taxon(id, name));
		map(name, id, err);
	    }
	    else {
		const char* c = Parse::trimr(a, d);
		d++;
		const char* f = Parse::find(d, b, '(', ')');
		const char* e = Parse::non_ws(d, f);
		/* " common name  (scientific name) ..."
		 *   a          c  d         e    f
		 */
		if(!genera.seen(d, e)) {
		    /* the genus is a taxon, too */
		    const TaxonId id(n++);
		    const std::string latin(d, e-d);
		    const std::string name = latin + " sp";
		    v.push_back(Taxon(id, name, latin));
		    map(name, id, err);
		    map(latin, id, err);
		}
		const TaxonId id(n++);
		const std::string name(a, c-a);
		const std::string latin(d, f-d);
		if(name=="-") {
		    v.push_back(Taxon(id, latin));
		    map(latin, id, err);
		}
		else {
		    v.push_back(Taxon(id, name, latin));
		    map(name, id, err);
		    map(latin, id, err);
		}
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


/**
 * Dump the taxon list, for debugging purposes.
 */
std::ostream& Taxa::put(std::ostream& os) const
{
    for(std::vector<Taxon>::const_iterator i = v.begin(); i!=v.end(); i++) {
	os << *i << '\n';
    }

    return os;
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
    s += "/lib/groblad/species";
    return s;
}


/**
 * Helper. Create the mapping from 'name' to taxon, and warn if this
 * doesn't work (a mapping already exists).
 */
void Taxa::map(const std::string& name, TaxonId id, std::ostream& err)
{
    if(!m.insert(std::make_pair(name, id)).second) {
	err << "warning: \"" << name << "\" has already been used to name a different taxon\n";
    }
}
