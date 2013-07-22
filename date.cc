/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "date.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <time.h>


Date::Date(const char* a, const char* b)
    : val(0)
{
    while(a!=b && !std::isdigit(*a)) a++;

    /* (pretending to have guarantees that
     * *b is a readable non-digit)
     */
    char* end;
    unsigned long n = std::strtoul(a, &end, 10);
    if(end-a == 4+2+2) {
	val = n;
    }
    else if(end-a == 2+2+2) {
	if(n<780101) {
	    /* y2k */
	    n+=1000000;
	}
	n += 19000000;
	val = n;
    }
    else if(end-a == 4) {
	val = n*10000;
	a = end;
	if(a==b || *a!='-') return;

	a++;
	n = std::strtoul(a, &end, 10);
	if(end-a != 2 || n<1 || n>12) return;
	val += n*100;
	a = end;
	if(a==b || *a!='-') return;

	a++;
	n = std::strtoul(a, &end, 10);
	if(end-a != 2 || n<1 || n>31) return;
	val += n;
    }
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
	std::strcpy(buf, "nil");
    }

    return os << buf;
}
