/*
 * Copyright (c) 2018 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "names.h"
#include "taxa.h"
#include "lineparse.h"

#include <cctype>
#include <cstring>
#include <vector>


namespace {

    using Range = Names::Range;

    /**
     * Is 'b' a prefix of 'a'?
     */
    bool prefix(const std::string& a, const std::string& b)
    {
	if(b.size() > a.size()) return false;
	auto i = begin(a);
	for(char ch: b) {
	    if(ch != *(i++)) return false;
	}
	return true;
    }

    /**
     * True if the sub-range 'c' of 's' is "a word" in some sense.
     * For now, that just means it's not adjacent to ASCII letters.
     * E.g. "foo" is not delimited in "foobar" or "barfoo".
     */
    bool delimited(const Range& s, const Range& c)
    {
	auto letter = [] (char ch) {
			  bool lower = 'a' <= ch && ch <= 'z';
			  bool upper = 'A' <= ch && ch <= 'Z';
			  return upper || lower;
		      };
	bool left = c.a==s.a || !letter(*(c.a-1));
	bool right = c.b==s.b || !letter(*(c.b));
	return left && right;
    }
}

/**
 * Find the first, longest name in s, or return an empty range
 * placed at the end of s.  Finds only delimited() names.
 */
Range Names::find_one(const Range s) const
{
    if(s.empty()) return s;

    Range n{s.b, s.b};
    Range c{s.a, s.a+1};

    while(c.b < s.b + 1) {
	const std::string cs = c.str();
	const auto i = names.lower_bound(cs);
	if(i!=names.end() && prefix(*i, cs)) {
	    if(delimited(s, c) && *i == cs) n = c;
	    c.b++;
	}
	else {
	    if(!n.empty()) break;
	    c = {c.a + 1, c.a + 2};
	}
    }
    return n;
}

/**
 * Find all occurrencies of 'names' in 's'.  Except overlaps: the
 * largest one is chosen.
 *
 * Returns the borders between names and non-names as a vector of iterators.
 * For example:
 *
 * ....name....    name.....
 * a   b   c   d   a   c    d
 *                 b
 *
 * So, N names means 2*(N+1) iterators.
 */
std::vector<const char*> Names::find(const std::string& s) const
{
    std::vector<const char*> acc;
    Range r{s.data(), s.data() + s.size()};

    while(true) {
	Range n = find_one(r);
	acc.push_back(r.a);
	acc.push_back(n.a);
	r.a = n.b;
	if(n.empty()) break;
    }

    return acc;
}
