/*
 * Copyright (C) 2014 Jörgen Grahn.
 * All rights reserved.
 */
#include <taxa.h>
#include <sstream>

#include <testicle.h>

namespace {

    Taxa list(const char* src)
    {
	std::istringstream iss(src);
	std::ostringstream err;

	Taxa spp(iss, err);
	testicle::assert_eq(err.str(), "");
	return spp;
    }

    void assert_sp(Taxa::const_iterator i, const char* name, const char* latin)
    {
	testicle::assert_eq(i->name, name);
	testicle::assert_eq(i->latin, latin);
    }

    void assert_same(const Taxa& spp, const char* a, const char* b, const char* c = 0)
    {
	testicle::assert_(spp.find(a));
	testicle::assert_eq(spp.find(a), spp.find(b));
	if(c) testicle::assert_eq(spp.find(a), spp.find(c));
    }
}


namespace taxa {
    using testicle::TC;

    void empty(TC)
    {
	const Taxa spp = list("");
	testicle::assert_(spp.begin() == spp.end());
    }

    void two(TC)
    {
	const Taxa spp = list("bergek  (Quercus petraea)\n"
			      "skogsek (Quercus robur)");
	Taxa::const_iterator i = spp.begin();
	assert_sp(i++, "Quercus sp", "Quercus");
	assert_sp(i++, "bergek", "Quercus petraea");
	assert_sp(i++, "skogsek", "Quercus robur");
	testicle::assert_(i == spp.end());
    }

    void common_only(TC)
    {
	const Taxa spp = list("skogsek");
	Taxa::const_iterator i = spp.begin();
	assert_sp(i++, "skogsek", "");
	testicle::assert_(i == spp.end());
    }

    void scientific_only(TC)
    {
	const Taxa spp = list("- (Taraxacum hamosiforme)");
	Taxa::const_iterator i = spp.begin();
	assert_sp(i++, "Taraxacum sp", "Taraxacum");
	assert_sp(i++, "Taraxacum hamosiforme", "Taraxacum hamosiforme");
	testicle::assert_(i == spp.end());
    }

    void alias(TC)
    {
	const Taxa spp = list("skogsek (Quercus robur)\n"
			      "= ek");
	Taxa::const_iterator i = spp.begin();
	assert_sp(i++, "Quercus sp", "Quercus");
	assert_sp(i++, "skogsek", "Quercus robur");
	testicle::assert_(i == spp.end());

	assert_same(spp, "skogsek", "ek", "Quercus robur");
    }

    void explicit_genus(TC)
    {
	const Taxa spp = list("-             (Cotoneaster)\n"
			      "starr         (Carex)\n"
			      "finnstarr     (Carex atherodes)\n"
			      "fetblad       (Phedimus)\n"
			      "= Sedum\n"
			      "gyllenfetblad (Phedimus aizoon)");
	Taxa::const_iterator i = spp.begin();
	assert_sp(i++, "Cotoneaster sp", "Cotoneaster");
	assert_sp(i++, "starr", "Carex");
	assert_sp(i++, "finnstarr", "Carex atherodes");
	assert_sp(i++, "fetblad", "Phedimus");
	assert_sp(i++, "gyllenfetblad", "Phedimus aizoon");
	testicle::assert_(i == spp.end());

	assert_same(spp, "Cotoneaster", "Cotoneaster sp");
	assert_same(spp, "starr", "Carex", "Carex sp");
	assert_same(spp, "fetblad", "Phedimus", "Phedimus sp");
	assert_same(spp, "fetblad", "Sedum");
    }
}
