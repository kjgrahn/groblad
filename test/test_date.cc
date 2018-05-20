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

    void assert_in_may(const Date& d)
    {
	orchis::assert_lt(D("2013-04-30"), d);
	orchis::assert_lt(d, D("2013-06-01"));
    }

    void assert_not_may(const Date& d)
    {
	bool a = D("2013-04-30") < d;
	bool b = d < D("2013-06-01");
	orchis::assert_false(a && b);
    }
}


namespace date {

    using orchis::assert_eq;
    using orchis::assert_lt;
    using orchis::TC;

    void simple(TC)
    {
	assert_in_may(D("2013-05-01"));
	assert_in_may(D("2013-05-31"));

	assert_not_may(D("2013-04-30"));
	assert_not_may(D("2013-06-01"));
    }

    void old_formats(TC)
    {
	assert_in_may(D("20130501"));
	assert_in_may(D("130501"));

	assert_not_may(D("20130601"));
	assert_not_may(D("130601"));
    }

    void extra_crud(TC)
    {
	assert_in_may(D("2013-05-01--"));
	assert_in_may(D("2013-05-01ff"));
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
	assert_lt(D("garbage"), D("1907"));
	assert_lt(D("19garbage"), D("1907"));
    }
}
