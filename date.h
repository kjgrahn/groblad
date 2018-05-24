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
#ifndef GAVIA_DATE_H
#define GAVIA_DATE_H

#include <iosfwd>
#include <string>

struct tm;

/**
 * A parsing of the Gavia date: header, mostly to provide ordering.
 * Since this header is free-form, with a few suggested date formats,
 * there are heuristics here.  The forms recognized:
 *
 * yymmdd     trailer
 * yyyymmdd   trailer
 * yyyy-mm-dd trailer
 * other
 *
 * There may be whitespace before the "true" date, or between the "true" date and
 * the trailer.  It's not required after yyyy-mm-dd though, or you wouldn't
 * want to write "2018-05-20--21".
 *
 * This class does preserve the actual text of the header, except for the
 * actual date formats which are normalized to ISO.
 */
class Date {
public:
    Date() : val(0) {}
    Date(const char* a, const char* b);
    bool operator< (const Date& other) const {
	if(val != other.val) return val<other.val;
	return trailer < other.trailer;
    }
    bool valid() const { return val; }

    struct tm tm() const;
    std::ostream& put(std::ostream& os) const;

private:
    unsigned val;
    std::string trailer;
};

inline
std::ostream& operator<< (std::ostream& os, const Date& date)
{
    return date.put(os);
}

#endif
