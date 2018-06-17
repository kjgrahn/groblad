/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "indent.h"

#include "utf8.h"

#include <iostream>
#include <algorithm>


namespace {

    /**
     * Indent by 'n', assuming 'm' positions have already been
     * written.
     *
     * <    m    >
     * <             n            >
     * 0       8       8       8       8
     * xxxxxxxxxxxT    T       ssss
     */
    std::ostream& spacer(std::ostream& os, size_t m, size_t n)
    {
	if(m < n) {
	    static const char SP[] = "        ";
	    const unsigned tabs = n/8;
	    const unsigned spaces = n%8;
	    unsigned consumed_tabs = m/8;
	    if(consumed_tabs < tabs) {
		while(consumed_tabs < tabs) {
		    os.put('\t');
		    consumed_tabs++;
		}
		os.write(SP, spaces);
	    }
	    else {
		os.write(SP, n-m);
	    }
	}
	return os;
    }

    struct Counter {
	struct Iterator {
	    explicit Iterator(Counter& c) : c(c) {}
	    Iterator& operator++(int) { c.n++; return *this; }
	    unsigned& operator* () { return c.dummy; }
	private:
	    Counter& c;
	};
	size_t n = 0;
	Iterator begin() { return Iterator{*this}; }
    private:
	unsigned dummy;
    };
}


/**
 * The width of 's', as per utf8::decode.  If a decoding error happens
 * (the string is e.g. iso8859-1) there's a permanent fallback to
 * plain string::size().
 */
size_t Indent::measure(const std::string& s)
{
    if(!utf8) return s.size();

    Counter counter;
    auto q = utf8::decode(begin(s), end(s), counter.begin());
    if (q != end(s)) {
	utf8 = false;
	return s.size();
    }
    return counter.n;
}


/**
 * Print 's' followed by enough whitespace to make the width 'n'.
 * TABs are used as far as possible. For this, it's assumed that
 * the output starts at column 0.
 *
 * <     n       >
 * foo............
 */
std::ostream& Indent::ljust(std::ostream& os, const std::string& s,
			    const size_t n)
{
    const size_t ss = measure(s);
    return spacer(os << s, ss, n);
}


/**
 * Print 's' preceded by enough whitespace to make the width 'n'.
 * TABs are used as far as possible. For this, it's assumed that
 * the output starts at column 0.
 *
 * <     n       >
 * ............foo
 */
std::ostream& Indent::rjust(std::ostream& os, const std::string& s,
			    const size_t n)
{
    const size_t ss = measure(s);
    if(ss < n) {
	spacer(os, 0, n - ss);
    }
    return os << s;
}


/**
 * Print the string 's', replacing any newlines with a newline and
 * an indentation (consisting of spaces and TABs) of width 'n'.
 *
 * <     n       >
 *                foo
 * ...............bar
 * ...............baz
 */
std::ostream& Indent::andjust(std::ostream& os, const std::string& s,
			      const size_t n) const
{
    const char* a = s.c_str();
    const char* const b = a + s.size();

    while(a!=b) {
	const char* p = std::find(a, b, '\n');
	if(p==b) {
	    os.write(a, p-a);
	    a = p;
	}
	else {
	    p++;
	    os.write(a, p-a);
	    spacer(os, 0, n);
	    a = p;
	}
    }
    return os;
}
