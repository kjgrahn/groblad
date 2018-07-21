/*----------------------------------------------------------------------------
 *
 * $Id: contstream.hh,v 1.5 2008-01-03 12:26:57 grahn Exp $
 *
 * contstream.h
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
 *----------------------------------------------------------------------------
 *
 * Like std::getline(std::istream&, std::string&), but with
 * continuation lines or folding:
 * - \n doesn't end lines, unless it's immediately followed by
 *   a non-blank character
 * - except completely blank lines are always themselves
 * - the logical lines have a single SP where the \s*\n\s+
 *   sequences used to be
 *
 * The state machine defines these events:
 * SP  - a blankspace line
 * SPX - a non-blank line starting with whitespace
 * X - a line starting with non-whitespace
 * EOF - end of file
 */

#ifndef CONTSTREAM_HH
#define CONTSTREAM_HH

#include <iosfwd>
#include <string>

/**
 * Wrapper around std::istream, whose only purpose is to support
 * getline() by keeping extra line state.
 *
 * The only normal std::istream stuff exposed is operator void*(),
 * for checking for EOF or errors in e.g. while loops.
 */
class Continuation {
public:
    explicit Continuation(std::istream& is)
	: is_(is),
	  has_acc_(false),
	  end_(false),
	  n_(0),
	  retn_(0)
    {}

    void getline(std::string& s);
    int line() const;
    operator void*() const;

private:
    Continuation();
    Continuation(const Continuation&);
    Continuation& operator= (const Continuation&);

    std::istream& is_;
    bool has_acc_;
    bool end_;
    int n_;
    int retn_;
    std::string acc_;
};


inline Continuation& getline(Continuation& conts, std::string& s)
{
    conts.getline(s);
    return conts;
}

#endif
