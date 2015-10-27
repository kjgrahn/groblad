/* Copyright (c) 2014 Jörgen Grahn
 * All rights reserved.
 */
#include <orchis.h>

#include <coordinate.h>

namespace {

    Coordinate coordinate(const std::string& s)
    {
	const char* p = s.c_str();
	return Coordinate(p, p+s.size());
    }
}

namespace coord {

    using orchis::TC;

    void broken(const std::string& s)
    {
	orchis::assert_(!coordinate(s).valid());
    }

    void broken(TC)
    {
	broken("");
	broken(" ");
	broken("a 6448794 1370358");
	broken("64487941370358");
	broken("1370358 6448794");
	broken("6448794 1370358a");
	broken("01370358 06448794");
    }

    void broken2(TC)
    {
	broken("6448794 1370358 a");
    }


    namespace rt90 {

	void valid(const std::string& s, unsigned res)
	{
	    const Coordinate c = coordinate(s);
	    orchis::assert_(c.valid());
	    orchis::assert_(c.rt90());
	    orchis::assert_eq(c.resolution, res);
	}

	void valid(TC)
	{
	    valid("6448794 1370358", 5);
	    valid("644879 137035", 10);
	    valid("64487 13703", 100);
	    valid("6448 1370", 1000);

	    valid("  6448 1370", 1000);
	    valid("  6448 1370  ", 1000);
	    valid("  6448  1370", 1000);
	    valid("  6448  1370  ", 1000);
	}

	void southwest(TC)
	{
	    valid("6100000 1200000", 5);
	    valid("610000 120000", 10);
	    valid("61000 12000", 100);
	    valid("6100 1200", 1000);
	}

	void northeast(TC)
	{
	    valid("7699999 1899999", 5);
	    valid("769999 189999", 10);
	    valid("76999 18999", 100);
	    valid("7699 1899", 1000);
	}

	void out_of_range(TC)
	{
	    broken("60999 12000");
	    broken("61000 11999");
	    broken("77000 18999");
	    broken("76999 19000");
	}
    }


    namespace sweref99 {

	void valid(const std::string& s)
	{
	    const Coordinate c = coordinate(s);
	    orchis::assert_(c.valid());
	    orchis::assert_(!c.rt90());
	    orchis::assert_eq(c.resolution, 5);
	}

	void valid(TC)
	{
	    valid("7454204 761811");
	    valid("6438299 410336");
	    valid("7104691 728831");
	}

	void out_of_range(TC)
	{
	    broken("761811 7454204");
	    broken("5454204 761811");
	    broken("7454204 61811");
	}
    }
}
