/*
 * Copyright (C) 2013 Jörgen Grahn.
 * All rights reserved.
 */
#include <filetest.h>

#include <orchis.h>


namespace {

    void readable(const char* f) {
	orchis::assert_eq(filetest::readable(f), true);
    }

    void not_readable(const char* f) {
	orchis::assert_eq(filetest::readable(f), false);
    }

    void writeable(const char* f) {
	orchis::assert_eq(filetest::writeable(f), true);
    }

    void not_writeable(const char* f) {
	orchis::assert_eq(filetest::writeable(f), false);
    }
}


namespace ftest {

    using orchis::TC;

    void readable(TC)
    {
	::readable("/etc/passwd");
    }

    void not_readable(TC)
    {
	::not_readable("/etc/shadow");
	::not_readable(".");
	::not_readable("/");
	::not_readable("xyzzy");
	::not_readable("");
    }
}
