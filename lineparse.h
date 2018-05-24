/* -*- c++ -*-
 *
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
#ifndef GAVIA_LINEPARSE_H
#define GAVIA_LINEPARSE_H

#include <cctype>

namespace Parse {

    inline bool isspace(char ch)
    {
	return std::isspace(static_cast<unsigned char>(ch));
    }

    inline bool isdigit(char ch)
    {
	return std::isdigit(static_cast<unsigned char>(ch));
    }

    /**
     * Trim whitespace to the left in [a, b).
     */
    inline
    const char* ws(const char* a, const char* b)
    {
	while(a!=b && isspace(*a)) a++;
	return a;
    }

    /**
     * Trim non-whitespace to the left in [a, b).
     */
    inline
    const char* non_ws(const char* a, const char* b)
    {
	while(a!=b && !isspace(*a)) a++;
	return a;
    }

    inline
    const char* digit(const char* a, const char* b)
    {
	while(a!=b && isdigit(*a)) a++;
	return a;
    }

    /**
     * Trim whitespace to the right in [a, b), so that
     * it's either empty or ends with non-whitespace.
     */
    inline
    const char* trimr(const char* a, const char* b)
    {
	while(a!=b && isspace(*(b-1))) b--;
	return b;
    }

    /**
     * Like std::find(), but finds right brackets, while ignoring
     * earlier left-right bracket pairs.  I.e. the one after "baz" is
     * found in "foo (bar) baz) bat)".
     */
    inline
    const char* find(const char* a, const char* const b,
		     const char left, const char right)
    {
	int level = 1;
	while(a!=b) {
	    if(*a==left) level++;
	    else if(*a==right) level--;
	    if(!level) break;
	    a++;
	}
	return a;
    }
}

#endif
