/* -*- c++ -*-
 *
 * Copyright (c) 2018 Jörgen Grahn
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
#ifndef GROBLAD_NAMES_H
#define GROBLAD_NAMES_H

#include <string>
#include <vector>
#include <set>


/**
 * Finding all occurrencies of names in strings, with features like:
 * - longest match rules
 * - case is significant, but capitalized versions are also found (for A-Z anyway)
 * - matches inside words are not found
 * - names may contain whitespace
 */
class Names {
public:
    template <class It> Names(It begin, It end);
    template <class Cont>
    explicit Names(const Cont& c)
	: Names{begin(c), end(c)}
    {}

    std::vector<const char*> find(const std::string& s) const;

    struct Range {
	const char* a;
	const char* b;
	bool empty() const { return a==b; }
	std::string str() const { return {a, b}; }
    };

private:
    std::set<std::string> names;

    Range find_one(const Range s) const;
};


template <class It>
Names::Names(It begin, It end)
{
    auto capitalize = [] (std::string& s) {
			  if(s.empty()) return false;
			  char ch = s[0];
			  bool lower = 'a' <= ch && ch <= 'z';
			  if(!lower) return false;
			  s[0] -= 'a' - 'A';
			  return true;
		      };

    It i = begin;
    while(i!=end) {
	std::string s = *i++;
	names.insert(s);
	if(capitalize(s)) names.insert(s);
    }
}

#endif
