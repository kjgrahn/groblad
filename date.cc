/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "date.h"
#include "lineparse.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <time.h>

namespace {
    const char* dash(const char* a, const char* b)
    {
	if(a!=b && *a=='-') a++;
	return a;
    }
}


Date::Date(const char* a, const char* const b)
    : val(0)
{
    auto a0 = Parse::ws(a, b);
    a = a0;

    auto c = Parse::digit(a, b);
    // [a, c) is a leading group of digits
    if(c-a == std::strlen("yyyymmdd")) {
	val = std::strtoul(a, nullptr, 10);
    }
    else if(c-a == std::strlen("yymmdd")) {
	unsigned n = std::strtoul(a, nullptr, 10);
	if(n<780101) {
	    /* y2k */
	    n += 1000000;
	}
	n += 19000000;
	val = n;
    }
    else if(c-a == std::strlen("yyyy")) {
	unsigned y = std::strtoul(a, nullptr, 10);
	a = dash(c, b);
	c = Parse::digit(a, b);
	if(c-a == std::strlen("mm")) {
	    unsigned m = std::strtoul(a, nullptr, 10);
	    a = dash(c, b);
	    c = Parse::digit(a, b);
	    if(c-a == std::strlen("dd")) {
		unsigned d = std::strtoul(a, nullptr, 10);
		val = (y * 100 + m) * 100 + d;
	    }
	    else {
		val = (y * 100 + m) * 100;
	    }
	}
	else {
	    val = y * 10000;
	}
    }
    else {
	c = a0;
    }

    trailer = {c, b};
}


/**
 * Conversion to a struct tm, expressed in local time.
 * (UTC is not quite possible, since Date carries no
 * time zone information.)
 *
 * No time of day is more likely than any other, so
 * we choose midnight.
 *
 * Date is also more flexible than a struct tm in that it can express
 * only year (2013-00-00) or only year and month (2013-07-00).  Such
 * Dates have to, unfortunately, be mapped to something unrelated; I
 * choose the Epoch.
 *
 * Day-of-week and day-of-year are not initialized.
 */
struct tm Date::tm() const
{
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_wday = 0;
    t.tm_yday = 0;
    t.tm_isdst = -1;

    t.tm_mday = val % 100;
    t.tm_mon  = (val/100) % 100 - 1;
    t.tm_year = val/10000 - 1900;

    if(t.tm_mday==0 || t.tm_mon==-1 || val==0) {
	t.tm_mday = 1;
	t.tm_mon  = 0;
	t.tm_year = 70;
    }

    return t;
}


std::ostream& Date::put(std::ostream& os) const
{
    char buf[4+3+3+1];
    const unsigned yyyy = val/10000;
    const unsigned mm   = (val/100) % 100;
    const unsigned dd   = val % 100;

    if(dd) {
	std::sprintf(buf, "%04u-%02u-%02u", yyyy, mm, dd);
    }
    else if(mm) {
	std::sprintf(buf, "%04u-%02u", yyyy, mm);
    }
    else if(yyyy) {
	std::sprintf(buf, "%04u", yyyy);
    }
    else {
	std::strcpy(buf, "");
    }

    return os << buf << trailer;
}
