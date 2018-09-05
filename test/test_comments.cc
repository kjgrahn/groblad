/*
 * Copyright (C) 2018 Jörgen Grahn.
 * All rights reserved.
 */
#include <comments.h>
#include <taxa.h>

#include <array>

#include <orchis.h>

namespace {

    static const std::array<std::string, 4> taxa {"mindre guldvinge",
						  "Lycaena phlaeas",
						  "videfuks",
						  "Nymphalis xanthomelas"};

    void assert_parses(const std::string& ref)
    {
	std::string s;
	std::remove_copy(begin(ref), end(ref),
			 std::back_inserter(s), '/');
	const auto pc = comment::parse(begin(taxa), end(taxa), s);

	orchis::assert_ge(pc.size(), 2);
	orchis::assert_eq(pc.size() % 2, 0);

	std::string acc;
	auto a = begin(pc);
	auto b = a+1;
	while(b!=end(pc)) {
	    if(a!=begin(pc)) acc.push_back('/');
	    acc.append(*a, *b);
	    a = b;
	    b++;
	}

	orchis::assert_eq(ref, acc);
    }
}


namespace comments {
    using orchis::TC;

    void nil(TC)
    {
	assert_parses("");
    }

    void no_taxa(TC)
    {
	assert_parses("foo");
	assert_parses("foo\n"
		      "bar");
	assert_parses("foo mindre");
	assert_parses("mindre foo");
    }

    void whole_words(TC)
    {
	assert_parses("videfuksx");
	assert_parses("svidefuks");
    }

    void one_taxon(TC)
    {
	assert_parses("/videfuks/");
	assert_parses(" /videfuks/   ");
	assert_parses("/videfuks/ foo");
	assert_parses("/videfuks/: foo");

	assert_parses("foo /videfuks/");
	assert_parses("foo:/videfuks/");
    }

    void two_taxa(TC)
    {
	assert_parses("foo /videfuks/ bar /videfuks/ baz");
    }

    void whitespace_taxon(TC)
    {
	assert_parses("/mindre guldvinge/");
	assert_parses("/mindre guldvinge/ foo");
	assert_parses("/mindre guldvinge/: foo");

	assert_parses("foo /mindre guldvinge/");
	assert_parses("foo:/mindre guldvinge/");
    }

    void capitalization(TC)
    {
	assert_parses("/mindre guldvinge/ /Mindre guldvinge/");
	assert_parses("MINDRE GULDVINGE");
	assert_parses("Mindre Guldvinge");
    }

    void non_capitalization(TC)
    {
	assert_parses("lycaena phlaeas /Lycaena phlaeas/");
    }
}
