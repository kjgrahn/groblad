/*
 * Copyright (c) 2018 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "comments.h"
#include "taxa.h"
#include "lineparse.h"

#include <cctype>
#include <cstring>
#include <vector>


namespace {

    struct Range {
	const char* a;
	const char* b;
	bool empty() const { return a==b; }
	std::string str() const { return {a, b}; }
    };

    /**
     * Trim leading and trailing whitespace in a range.
     */
    std::string trim(const Range& s)
    {
	auto a = Parse::ws(s.a, s.b);
	auto b = Parse::trimr(a, s.b);
	return {a, b};
    }

    bool is_separator(char ch)
    {
	if(std::isspace(static_cast<unsigned>(ch))) return true;
	return std::strchr("!\"'()[],.:;/", ch);
    }

    const char* eats(const char* a, const char* b)
    {
	while(a!=b) {
	    if(!is_separator(*a)) return a;
	    a++;
	}
	return a;
    }

    const char* eatw(const char* a, const char* b)
    {
	while(a!=b) {
	    if(is_separator(*a)) return a;
	    a++;
	}
	return a;
    }

    std::vector<Range> find_taxa(const Taxa& taxa, const char* a, const char* b)
    {
	auto is_taxon = [taxa] (const Range& r) {
			    return taxa.find(r.str());
			};

	std::vector<Range> acc;

	while(a!=b) {
	    auto p = eats(a, b);
	    auto q = eatw(p, b);
	    const Range r{p, q};
	    if(is_taxon(r)) {
		acc.push_back(r);
		a = q;
	    }
	    else {
		auto t = eats(q, b);
		q = eatw(t, b);
		const Range r{p, q};
		if(is_taxon(r)) {
		    acc.push_back(r);
		    a = q;
		}
		else {
		    a = t;
		}
	    }
	}
	return acc;
    }
}

ParsedComment parse(const Taxa& taxa, const std::string& s)
{
    const char* a = s.data();
    const char* const b = a + s.size();

    const auto tt = find_taxa(taxa, a, b);
    if(tt.empty()) return {s, {}};

    ParsedComment pc;
    pc.pre = trim({a, tt.front().a});

    for(auto i = begin(tt); i!=end(tt); i++) {
	if(i!=begin(tt)) {
	    pc.sightings.back().comment = trim({a, i->a});
	}
	pc.sightings.push_back({i->str(), ""});
	a = i->b;
    }
    pc.sightings.back().comment = trim({tt.back().b, b});
    return pc;
}
