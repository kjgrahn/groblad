/**
 * $Id: contstream.cc,v 1.8 2008-01-03 12:51:38 grahn Exp $
 *
 * Copyright (c) 2006, 2008 Jörgen Grahn
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
 *
 */
static const char* rcsid() { rcsid(); return
"$Id: contstream.cc,v 1.8 2008-01-03 12:51:38 grahn Exp $";
}

#include <iostream>
#include "contstream.h"

namespace {
    /**
     * True iff s is empty or only contains space and/or TABs.
     */
    bool isblank(const std::string& s)
    {
	return s.find_first_not_of(" \t")==std::string::npos;
    }

    /**
     * True iff s contains characters, and starts with space or TAB.
     */
    bool starts_blank(const std::string& s)
    {
	return !s.empty() && (s[0]==' ' || s[0]=='\t');
    }

    /**
     * append 's' to 'acc', while replacing any trailing whitespace
     * of 'acc' and leading whitespace of 's' with a single ' '.
     */
    std::string& append(std::string& acc, const std::string& s)
    {
	std::string::size_type i = acc.find_last_not_of(" \t") + 1;
	acc.replace(i, acc.size() - i, " ");

	i = s.find_first_not_of(" \t");
	acc += s.substr(i);
	return acc;
    }
}

void Continuation::getline(std::string& s)
{
    std::string tmp;
    while(std::getline(is_, tmp)) {
	++n_;

	if(has_acc_) {
	    if(isblank(tmp) || !starts_blank(tmp)) {
		std::swap(acc_, s);
		std::swap(acc_, tmp);
		retn_ = n_-1;
		return;
	    }
	    else {
		append(acc_, tmp);
	    }
	}
	else {
	    if(!isblank(tmp)) {
		std::swap(acc_, tmp);
		has_acc_ = true;
	    }
	    else {
		std::swap(tmp, s);
		retn_ = n_;
		return;
	    }
	}
    }

    if(has_acc_) {
	std::swap(acc_, s);
	retn_ = n_;
	has_acc_ = false;
    }
    else {
	end_ = true;
    }

#if 0
    while(is_) {
	std::string tmp;
	std::getline(is_, tmp);
	++n_;

	if(starts_blank(tmp) && !isblank(tmp)) {
	    append(acc_, tmp);
	}
	else if(has_acc_) {
	    s = acc_;
	    acc_ = tmp;
	    return;
	}
	else {
	    acc_ = tmp;
	    has_acc_ = true;
	}
    }

    s = acc_;
#endif
}

/**
 * The number of the most recently read physical line
 * (the first line being line 1).
 */
int Continuation::line() const
{
    return retn_;
}


Continuation::operator void*() const
{
    if(end_) {
	return 0;
    }

    return const_cast<Continuation *>(this);
}
