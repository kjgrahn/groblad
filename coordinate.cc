/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "coordinate.h"

#include <cctype>
#include <cstdlib>


namespace {

    bool in_range(unsigned a, unsigned b, unsigned c)
    {
	return a < b && b < c;
    }
}


Coordinate::Coordinate(const char* a, const char* b)
    : north(0),
      east(0),
      resolution(1)
{
    const char* p = a;
    while(p!=b && std::isdigit(*p)) p++;
    if(p-a > 7) return;
    if(p-a < 4) return;
    while(p!=b && std::isspace(*p)) p++;

    const char* const c = p;
    while(p!=b && std::isdigit(*p)) p++;
    if(p-c > 7) return;
    if(p-c < 4) return;
    if(p!=b && !std::isspace(*p)) return;

    north = std::strtoul(a, 0, 10);
    east = std::strtoul(c, 0, 10);

    while(north < 1000000) {
	north *= 10;
	east *= 10;
	resolution *= 10;
    }

    if(!in_range(6100000, north, 7800000) ||
       !in_range(1200000, east,  1900000)) {
	north = east = 0;
    }

    if(resolution==1) {
	/* Artportalen doesn't believe in 1m resolution */
	resolution = 5;
    }
}
