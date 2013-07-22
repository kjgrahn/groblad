/* -*- c++ -*-
 *
 * $Id: md5pp.h,v 1.4 2011-02-06 15:05:54 grahn Exp $
 *
 * Copyright (c) 2010 Jörgen Grahn <grahn+src@snipabacken.se>
 * All rights reserved.
 */
#ifndef GEESE_MD5_H
#define GEESE_MD5_H

#include <iosfwd>
#include <string>
#include <vector>

#include "md5.h"

namespace md5 {

    /**
     * Just a wrapping of the 16-octet MD5 digest. The C API
     * should have had one, too.
     */
    struct Digest {
	Digest() {}
	explicit Digest(const unsigned char buf[16]);
	std::string hex() const;
	bool operator== (const Digest& other) const;
	bool operator!= (const Digest& other) const { return !(*this==other); }
	unsigned char val[16];
    };


    /**
     * Like Solar Designer's MD5_CTX, but with a richer set of input
     * types, and retrieving the digest doesn't destroy the context
     * state. Like MD5_CTX, it's copyable.
     */
    class Ctx {
    public:
	Ctx();

	Ctx& update(const char* data, unsigned long size);
	Ctx& update(const unsigned char* data, unsigned long size);

	Ctx& update(const std::string& data);
	Ctx& update(const std::vector<char>& data);
	Ctx& update(const std::vector<unsigned char>& data);
	Ctx& update(std::istream& in);

	template<class Iter>
	Ctx& update(Iter a, Iter b);

	Digest digest() const;

    private:
	MD5_CTX val;
    };


    template<class Iter>
    Ctx& Ctx::update(Iter a, Iter b)
    {
	while(a!=b) {
	    unsigned char buf[64];
	    unsigned char* p = buf;

	    while(a!=b && p!=buf+sizeof buf) {
		*p = *a;
		++a;
		++p;
	    }
	    update(buf, p-buf);
	}
	return *this;
    }

    std::ostream& operator<< (std::ostream& os, const Digest& val);
}

#endif
