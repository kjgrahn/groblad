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
#include "regex.h"

#include <sys/types.h>
#include <regex.h>


/**
 * Needed only because POSIX regexes don't allow for forward
 * declarations -- there is no struct regex, only a regex_t typedef.
 */
struct Regex::Wrapper {
    explicit Wrapper(const std::string& regex) {
	err = regcomp(&re, regex.c_str(),
		      REG_EXTENDED|
		      REG_ICASE|
		      REG_NOSUB);
    }
    ~Wrapper() {
	if(!err) regfree(&re);
    }
    int err;
    regex_t re;
};


Regex::Regex(const std::string& regex)
    : wrapper(new Wrapper(regex))
{}


Regex::~Regex()
{
    delete wrapper;
}


/**
 * True if the Regex is broken, i.e. unsuitable for a match().
 */
bool Regex::bad() const
{
    return wrapper->err;
}


/**
 * If bad(), the error string (or at least an initial part of it).
 */
std::string Regex::error() const
{
    char buf[60];
    regerror(wrapper->err, &wrapper->re, buf, sizeof buf);
    return buf;
}


bool Regex::match(const char* s) const
{
    return !regexec(&wrapper->re, s, 0, 0, 0);
}
