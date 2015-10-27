/*
 * Copyright (C) 2013 Jörgen Grahn.
 * All rights reserved.
 */
#include <date.h>

#include <orchis.h>

namespace {

    Date D(const std::string& s)
    {
	const char* p = s.c_str();
	return Date(p, p+s.size());
    }
}


namespace date {

    using orchis::assert_eq;
    using orchis::assert_lt;
    using orchis::TC;

    void simple(TC)
    {
	assert_eq(D("2013-05-01"), D("2013-05-01"));
	assert_lt(D("2013-05-18"), D("2013-05-19"));
    }

    void crazy(TC)
    {
	assert_eq(D("2013-05-41"), D("2013-05"));
	assert_eq(D("2013-15-01"), D("2013"));
    }

    void old_formats(TC)
    {
	assert_eq(D("2013-05-01"), D("20130501"));
	assert_eq(D("2013-05-01"), D("130501"));
    }

    void extra_crud(TC)
    {
	assert_eq(D("2013-05-01"), D("2013-05-01--"));
	assert_eq(D("2013-05-01"), D("2013-05-01ff"));
    }

    void missing_stuff(TC)
    {
	assert_lt(D("2013-05"), D("2013-05-01"));
	assert_lt(D("2013-04-30"), D("2013-05"));

	assert_lt(D("2013"), D("2013-01-01"));
	assert_lt(D("2012-12-31"), D("2013"));
    }

    void empty(TC)
    {
	assert_lt(D(""), D("1907"));
	assert_eq(D(""), D("garbage"));
	assert_eq(D(""), D("19garbage"));
    }
}
