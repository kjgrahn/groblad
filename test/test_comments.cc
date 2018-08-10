/*
 * Copyright (C) 2018 Jörgen Grahn.
 * All rights reserved.
 */
#include <comments.h>
#include <taxa.h>

#include <array>

#include <orchis.h>

namespace {

    std::istringstream taxa_stream("mindre guldvinge (Lycaena phlaeas)\n"
				   "videfuks (Nymphalis xanthomelas)\n");
    std::ostringstream devnull;
    static const Taxa taxa(taxa_stream, devnull);
    static const std::array<std::string, 2> taxarr {"mindre guldvinge",
						    "videfuks"};

    template <class C>
    void assert_empty(const C& c)
    {
	orchis::assert_true(c.empty());
    }

    void assert_no_taxa(const char* s)
    {
	auto pc = parse(taxa, s);
	orchis::assert_eq(pc.pre, s);
	assert_empty(pc.sightings);
    }

    void assert_one_taxon(const char* s,
			  const char* pre, const char* comment)
    {
	auto pc = parse(taxa, s);
	orchis::assert_eq(pc.pre, pre);
	orchis::assert_eq(pc.sightings.size(), 1);
	const auto& sighting = pc.sightings.front();
	orchis::assert_in(taxarr, sighting.name);
	orchis::assert_eq(sighting.comment, comment);
    }

    void assert_two_taxa(const char* s,
			 const char* pre, const char* c0, const char* c1)
    {
	auto pc = parse(taxa, s);
	orchis::assert_eq(pc.pre, pre);
	orchis::assert_eq(pc.sightings.size(), 2);
	orchis::assert_in(taxarr, pc.sightings[0].name);
	orchis::assert_in(taxarr, pc.sightings[1].name);
	orchis::assert_eq(pc.sightings[0].comment, c0);
	orchis::assert_eq(pc.sightings[1].comment, c1);
    }
}


namespace comments {
    using orchis::TC;

    void nil(TC)
    {
	auto pc = parse(taxa, "");
	assert_empty(pc.pre);
	assert_empty(pc.sightings);
    }

    void no_taxa(TC)
    {
	assert_no_taxa("");
	assert_no_taxa("foo");
	assert_no_taxa("foo\n"
		       "bar");
	assert_no_taxa("foo mindre");
	assert_no_taxa("mindre foo");
	assert_no_taxa("videfuksx");
    }

    void one_taxon(TC)
    {
	assert_one_taxon("videfuks",      "", "");
	assert_one_taxon(" videfuks   ",  "", "");
	assert_one_taxon("videfuks foo",  "", "foo");
	assert_one_taxon("videfuks: foo", "", ": foo");

	assert_one_taxon("foo videfuks",  "foo", "");
	assert_one_taxon("foo:videfuks",  "foo:", "");
    }

    void two_taxa(TC)
    {
	assert_two_taxa("foo videfuks bar videfuks baz",
			"foo", "bar", "baz");
    }

    void whitespace_taxon(TC)
    {
	assert_one_taxon("mindre guldvinge",      "", "");
	assert_one_taxon("mindre guldvinge foo",  "", "foo");
	assert_one_taxon("mindre guldvinge: foo", "", ": foo");

	assert_one_taxon("foo mindre guldvinge",  "foo", "");
	assert_one_taxon("foo:mindre guldvinge",  "foo:", "");
    }
}
