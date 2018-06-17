/* Copyright (C) 2013 Jörgen Grahn.
 * All rights reserved.
 */
#include <indent.h>

#include <orchis.h>
#include <iostream>
#include <sstream>

using std::string;

namespace {

    string ljust(const std::string& s, const size_t n)
    {
	std::ostringstream oss;
	Indent indent;
	indent.ljust(oss, s, n);
	return oss.str();
    }

    string rjust(const std::string& s, const size_t n)
    {
	std::ostringstream oss;
	Indent indent;
	indent.rjust(oss, s, n);
	return oss.str();
    }

    string andjust(const std::string& s, const size_t n)
    {
	std::ostringstream oss;
	const Indent indent;
	indent.andjust(oss, s, n);
	return oss.str();
    }
}

namespace indent {

    using orchis::TC;
    using orchis::assert_eq;

    void simple(TC)
    {
	assert_eq(ljust("foo", 5), "foo  ");
	assert_eq(rjust("foo", 5), "  foo");
	assert_eq(andjust("foo\nbar\nbaz", 1),
		  "foo\n"
		  " bar\n"
		  " baz");
    }

    namespace measure {

	void null(TC)
	{
	    Indent in;
	    assert_eq(in.measure(""), 0);
	}

	void ascii(TC)
	{
	    Indent in;
	    assert_eq(in.measure("foo"), 3);
	}

	void latin1(TC)
	{
	    Indent in;
	    assert_eq(in.measure("na\xefve"), 5);
	}

	void utf8(TC)
	{
	    Indent in;
	    assert_eq(in.measure("\xc2\x80-\xef\xbf\xbf"), 3);
	}

	void fallback(TC)
	{
	    Indent in;
	    assert_eq(in.measure("\xc2\x80-\xef\xbf\xbf"), 3);
	    assert_eq(in.measure("na\xefve"), 5);
	    assert_eq(in.measure("\xc2\x80-\xef\xbf\xbf"), 6);
	}
    }

    namespace left {

	void test_0(TC)
	{
	    assert_eq(ljust("", 0), "");
	    assert_eq(ljust("", 1), " ");
	    assert_eq(ljust("", 2), "  ");
	    assert_eq(ljust("", 7), "       ");
	    assert_eq(ljust("", 8), "\t");
	    assert_eq(ljust("", 9), "\t ");
	    assert_eq(ljust("", 15), "\t       ");
	    assert_eq(ljust("", 16), "\t\t");
	}

	void test_1(TC)
	{
	    assert_eq(ljust("a", 1), "a");
	    assert_eq(ljust("a", 2), "a ");
	    assert_eq(ljust("a", 7), "a      ");
	    assert_eq(ljust("a", 8), "a\t");
	    assert_eq(ljust("a", 9), "a\t ");
	    assert_eq(ljust("a", 15), "a\t       ");
	    assert_eq(ljust("a", 16), "a\t\t");
	}

	void test_2(TC)
	{
	    assert_eq(ljust("ab", 2), "ab");
	    assert_eq(ljust("ab", 7), "ab     ");
	    assert_eq(ljust("ab", 8), "ab\t");
	    assert_eq(ljust("ab", 9), "ab\t ");
	    assert_eq(ljust("ab", 15), "ab\t       ");
	    assert_eq(ljust("ab", 16), "ab\t\t");
	}

	void test_7(TC)
	{
	    assert_eq(ljust("abcdefg", 7), "abcdefg");
	    assert_eq(ljust("abcdefg", 8), "abcdefg\t");
	    assert_eq(ljust("abcdefg", 9), "abcdefg\t ");
	    assert_eq(ljust("abcdefg", 15), "abcdefg\t       ");
	    assert_eq(ljust("abcdefg", 16), "abcdefg\t\t");
	}

	void test_8(TC)
	{
	    assert_eq(ljust("abcdefgh", 8), "abcdefgh");
	    assert_eq(ljust("abcdefgh", 9), "abcdefgh ");
	    assert_eq(ljust("abcdefgh", 15), "abcdefgh       ");
	    assert_eq(ljust("abcdefgh", 16), "abcdefgh\t");
	    assert_eq(ljust("abcdefgh", 17), "abcdefgh\t ");
	}

	void test_9(TC)
	{
	    assert_eq(ljust("abcdefghi", 9), "abcdefghi");
	    assert_eq(ljust("abcdefghi", 10), "abcdefghi ");
	    assert_eq(ljust("abcdefghi", 15), "abcdefghi      ");
	    assert_eq(ljust("abcdefghi", 16), "abcdefghi\t");
	    assert_eq(ljust("abcdefghi", 17), "abcdefghi\t ");
	}
    }

    namespace right {

	void test_0(TC)
	{
	    assert_eq(rjust("", 0), "");
	    assert_eq(rjust("", 1), " ");
	    assert_eq(rjust("", 2), "  ");
	    assert_eq(rjust("", 7), "       ");
	    assert_eq(rjust("", 8), "\t");
	    assert_eq(rjust("", 9), "\t ");
	    assert_eq(rjust("", 15), "\t       ");
	    assert_eq(rjust("", 16), "\t\t");
	}

	void test_1(TC)
	{
	    assert_eq(rjust("A", 1), "A");
	    assert_eq(rjust("A", 2), " A");
	    assert_eq(rjust("A", 7), "      A");
	    assert_eq(rjust("A", 8), "       A");
	    assert_eq(rjust("A", 9), "\tA");
	    assert_eq(rjust("A", 15), "\t      A");
	    assert_eq(rjust("A", 16), "\t       A");
	    assert_eq(rjust("A", 17), "\t\tA");
	}
    }
}
