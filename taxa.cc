/*
 * Copyright (c) 2013, 2014, 2020 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "taxa.h"
#include "lineparse.h"

#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <cassert>


namespace {

    /**
     * Just a helper class, for processing a genus name only once.
     */
    class Genera {
	std::unordered_set<std::string> s;
    public:
	bool seen(const std::string& name);
	bool seen(const char* a, const char* b);
    };

    bool Genera::seen(const std::string& name)
    {
	return !s.insert(name).second;
    }

    bool Genera::seen(const char* a, const char* b)
    {
	return seen(std::string(a, b-a));
    }

    std::string either(const std::string& a, const std::string& b)
    {
	if (a=="-") return b;
	return a;
    }

    std::string sp(std::string latin)
    {
	latin += " sp";
	return latin;
    }
}


/**
 * Create the list from a text file in the format documented
 * in groblad_species(5).
 *
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
		v.push_back(Taxon(id, false, name));
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
		if(e==f) {
		    /* genus */
		    const TaxonId id(n++);
		    std::string name(a, c-a);
		    const std::string latin(d, f-d);
		    const bool empty_name = name=="-";
		    Taxon genus(id, true, either(name, sp(latin)), latin);
		    if (!empty_name) {
			genus.add(latin + " sp");
		    }
		    genera.seen(latin);
		    map(genus, err);
		    v.push_back(genus);
		}
		else {
		    if(!genera.seen(d, e)) {
			/* the genus is a taxon, too */
			const TaxonId id(n++);
			const std::string latin(d, e-d);
			const Taxon genus(id, true, sp(latin), latin);
			map(genus, err);
			v.push_back(genus);
		    }
		    const TaxonId id(n++);
		    std::string name(a, c-a);
		    const std::string latin(d, f-d);
		    const Taxon sp(id, false, either(name, latin), latin);
		    map(sp, err);
		    v.push_back(sp);
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
	v.emplace_back(id, false, name);
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
    auto i = m.find(name);
    if(i!=m.end()) {
	id = i->second;
    }
    return id;
}


/**
 * All taxon names, aliases and all, in no particular order.
 */
std::vector<std::string> Taxa::names() const
{
    std::vector<std::string> acc;
    for(const auto& item: m) acc.push_back(item.first);
    return acc;
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
    for(const Taxon& sp: v) {
	if(sp.match(re)) acc.push_back(sp.id);
    }
    return acc;
}


/**
 * Dump the taxon list, for debugging purposes.
 */
std::ostream& Taxa::put(std::ostream& os) const
{
    for(const Taxon& sp: v) {
	os << sp << '\n';
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
 * Helper. Map all names in 'sp'.
 */
void Taxa::map(const Taxon& sp, std::ostream& err)
{
    map(sp.name, sp.id, err);
    if(!sp.latin.empty()) map(sp.latin, sp.id, err);

    for(const auto& name: sp.alias) map(name, sp.id, err);
}


/**
 * Helper. Create the mapping from 'name' to taxon, and warn if this
 * doesn't work (a different mapping already exists).
 */
void Taxa::map(const std::string& name, TaxonId id, std::ostream& err)
{
    auto p = m.insert(std::make_pair(name, id));
    if(!p.second && p.first->second != id) {
	err << "warning: \"" << name << "\" has already been used to name a different taxon\n";
    }
}
