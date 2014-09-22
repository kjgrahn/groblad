/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "coordinate.h"

#include <cctype>
#include <cstdlib>


namespace {

    /**
     * True iff b is in [a..c).
     */
    bool in_range(unsigned a, unsigned b, unsigned c)
    {
	return a <= b && b < c;
    }

    bool in_rt90(unsigned north, unsigned east)
    {
	if(!in_range(6100000, north, 7700000)) return false;
	return in_range(1200000, east,  1900000);
    }

    bool in_sweref99(unsigned north, unsigned east)
    {
	/* Unlike RT90 which has defined limits, I haven't found them
	 * for SWEREF99. The figures here are measured around the RT90
	 * edges.
	 */
	if(!in_range(6100000, north, 7700000)) return false;
	return in_range(250000, east, 925000);
    }
}


Coordinate::Coordinate(const char* a, const char* const b)
    : north(0),
      east(0),
      resolution(1)
{
    while(a!=b && std::isspace(*a)) a++;

    const char* p = a;
    while(p!=b && std::isdigit(*p)) p++;
    if(p-a > 7) return;
    if(p-a < 4) return;
    while(p!=b && std::isspace(*p)) p++;

    const char* const c = p;
    while(p!=b && std::isdigit(*p)) p++;
    if(p-c > 7) return;
    if(p-c < 4) return;

    while(p!=b && std::isspace(*p)) p++;
    if(p!=b) return;

    north = std::strtoul(a, 0, 10);
    east = std::strtoul(c, 0, 10);

    while(north < 1000000) {
	north *= 10;
	east *= 10;
	resolution *= 10;
    }

    if(!in_rt90(north, east)) {

	if(!in_sweref99(north, east) || resolution!=1) {
	    north = east = 0;
	}
    }

    if(resolution==1) {
	/* Artportalen doesn't believe in 1m resolution */
	resolution = 5;
    }
}
