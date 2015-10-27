/*
 * $Id: test_cont.cc,v 1.9 2008-01-03 12:51:39 grahn Exp $
 *
 * Copyright (C) 2006, 2007 Jörgen Grahn.
 * All rights reserved.
 */
#include <iostream>
#include <sstream>

#include <orchis.h>

#include <contstream.h>

using std::istringstream;
using std::string;

using orchis::TC;

namespace {
    std::string line(Continuation& c) {
	orchis::assert_true(c);
	string s("deadbeef");
	getline(c, s);
	return s;
    }

    void assert_line(Continuation& c,
		     const int lineno,
		     const char * const s) {
	std::string ss;
	orchis::assert_true(getline(c, ss));
	orchis::assert_eq(ss, s);
	orchis::assert_eq(c.line(), lineno);
    }

    void assert_eof(Continuation& c) {
	std::string s;
	orchis::assert_true(!getline(c, s));
    }
}

namespace cont {

    using orchis::assert_true;
    using orchis::assert_eq;

    void A(TC) {
	istringstream iss("");
	Continuation cs(iss);
	assert_eq("deadbeef", line(cs));
	assert_true(!cs);
    }
    void B(TC) {
	istringstream iss("\n");
	Continuation cs(iss);
	assert_eq("", line(cs));
	assert_eq(1, cs.line());
	assert_eof(cs);
    }
    void C(TC) {
	istringstream iss("foo");
	Continuation cs(iss);
	assert_eq("foo", line(cs));
	assert_eq(1, cs.line());
	assert_eof(cs);
    }
    void D(TC) {
	istringstream iss("foo\n");
	Continuation cs(iss);
	assert_eq("foo", line(cs));
	assert_eq(1, cs.line());
	assert_eof(cs);
    }
    void E(TC) {
	istringstream iss("foo ");
	Continuation cs(iss);
	assert_eq("foo ", line(cs));
	assert_eq(1, cs.line());
	assert_eof(cs);
    }
    void F(TC) {
	istringstream iss("foo\n"
			  "bar\n");
	Continuation cs(iss);
	assert_eq("foo", line(cs));
	assert_eq(1, cs.line());
	assert_eq("bar", line(cs));
	assert_eq(2, cs.line());
	assert_eof(cs);
    }
    void G(TC) {
	istringstream iss("foo bar\t\n"
			  "  foo bar\n");
	Continuation cs(iss);
	assert_eq("foo bar foo bar", line(cs));
	assert_eq(2, cs.line());
	assert_eof(cs);
    }
    void H(TC) {
	istringstream iss("foobar\t\n"
			  "  foobar \n"
			  "  foobar \n"
			  "foobar\n");
	Continuation cs(iss);
	assert_eq("foobar foobar foobar ", line(cs));
	assert_eq(3, cs.line());
	assert_eq("foobar", line(cs));
	assert_eq(4, cs.line());
	assert_eof(cs);
    }
    void I(TC) {
	istringstream iss("foobar\t\n"
			  "  foobar \n"
			  "\t\t\n"
			  "foobar\n");
	Continuation cs(iss);
	assert_eq("foobar foobar ", line(cs));
	assert_eq(2, cs.line());
	assert_eq("\t\t", line(cs));
	assert_eq(3, cs.line());
	assert_eq("foobar", line(cs));
	assert_eq(4, cs.line());
	assert_eof(cs);
    }
    void J(TC) {
	istringstream iss("foo\n"
			  " foo\n"
			  " foo\n"
			  " foo\n");
	Continuation cs(iss);
	assert_eq("foo foo foo foo", line(cs));
	assert_eq(4, cs.line());
	assert_eof(cs);
    }
    void K(TC) {
	istringstream iss("foo     \n"
			  "bar\n");
	Continuation cs(iss);
	assert_eq("foo     ", line(cs));
	assert_eq(1, cs.line());
	assert_eq("bar", line(cs));
	assert_eq(2, cs.line());
	assert_eof(cs);
    }
}


namespace continuation {

    void A(TC) {
	istringstream iss("");
	Continuation cs(iss);
	std::string s("deadbeef");
	orchis::assert_true(!getline(cs, s));
	orchis::assert_eq(s, "deadbeef");
    }
    void B(TC) {
	istringstream iss("\n");
	Continuation cs(iss);
	assert_line(cs, 1, "");
	assert_eof(cs);
    }
    void C(TC) {
	istringstream iss("%foo");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo");
	assert_eof(cs);
    }
    void D(TC) {
	istringstream iss("%foo\n");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo");
	assert_eof(cs);
    }
    void E(TC) {
	istringstream iss("%foo ");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo ");
	assert_eof(cs);
    }
    void F(TC) {
	istringstream iss("%foo\n"
			  "%bar\n");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo");
	assert_line(cs, 2, "%bar");
	assert_eof(cs);
    }
    void G(TC) {
	istringstream iss("%foo bar\t\n"
			  "  foo bar\n");
	Continuation cs(iss);
	assert_line(cs, 2, "%foo bar foo bar");
	assert_eof(cs);
    }
    void H(TC) {
	istringstream iss("%foobar\t\n"
			  "  foobar \n"
			  "  foobar \n"
			  "%foobar\n");
	Continuation cs(iss);
	assert_line(cs, 3, "%foobar foobar foobar ");
	assert_line(cs, 4, "%foobar");
	assert_eof(cs);
    }
    void I(TC) {
	istringstream iss("%foobar\t\n"
			  "  foobar \n"
			  "\n"
			  "%foobar\n");
	Continuation cs(iss);
	assert_line(cs, 2, "%foobar foobar ");
	assert_line(cs, 3, "");
	assert_line(cs, 4, "%foobar");
	assert_eof(cs);
    }
    void J(TC) {
	istringstream iss("%foo\n"
			  " foo\n"
			  " foo\n"
			  " foo\n");
	Continuation cs(iss);
	assert_line(cs, 4, "%foo foo foo foo");
	assert_eof(cs);
    }
    void K(TC) {
	istringstream iss("%foo     \n"
			  "%bar\n");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo     ");
	assert_line(cs, 2, "%bar");
	assert_eof(cs);
    }
    void L(TC) {
	istringstream iss("%foo \n"
			  " bar\t\n"
			  " baz \n"
			  "%foo \n"
			  " bar\t\n"
			  " baz \n"
			  " \n"
			  "%foo \n"
			  " bar\t\n"
			  " baz \n");
	Continuation cs(iss);
	assert_line(cs, 3, "%foo bar baz ");
	assert_line(cs, 6, "%foo bar baz ");
	assert_line(cs, 7, " ");
	assert_line(cs, 10, "%foo bar baz ");
	assert_eof(cs);
    }
    void M(TC) {
	istringstream iss("  \n"
			  "\t\t");
	Continuation cs(iss);
	assert_line(cs, 1, "  ");
	assert_line(cs, 2, "\t\t");
	assert_eof(cs);
    }
    void N(TC) {
	istringstream iss("  \n"
			  "%foo\n"
			  " bar\n"
			  " baz\n"
			  "%bat");
	Continuation cs(iss);
	assert_line(cs, 1, "  ");
	assert_line(cs, 4, "%foo bar baz");
	assert_line(cs, 5, "%bat");
	assert_eof(cs);
    }
    void O(TC) {
	istringstream iss("%foo\n"
			  " bar\n"
			  " baz\n"
			  "%bat\n"
			  " ben");
	Continuation cs(iss);
	assert_line(cs, 3, "%foo bar baz");
	assert_line(cs, 5, "%bat ben");
	assert_eof(cs);
    }
    void P(TC) {
	istringstream iss("%foo\n"
			  " foo\n"
			  "%foo\n"
			  " foo\n");
	Continuation cs(iss);
	assert_line(cs, 2, "%foo foo");
	assert_line(cs, 4, "%foo foo");
	assert_eof(cs);
    }
    void R(TC) {
	istringstream iss("%foo\n"
			  "\n"
			  "%bar");
	Continuation cs(iss);
	assert_line(cs, 1, "%foo");
	assert_line(cs, 2, "");
	assert_line(cs, 3, "%bar");
	assert_eof(cs);
	assert_eof(cs);
	assert_eof(cs);
    }
}
