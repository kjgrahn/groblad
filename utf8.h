/* -*- c++ -*-
 *
 * Copyright (c) 2017, 2018 Jörgen Grahn <grahn+src@snipabacken.se>
 * All rights reserved.
 */
#ifndef GAVIA_UTF8_H
#define GAVIA_UTF8_H

namespace utf8 {

    /**
     * Automaton for decoding of an UTF-8 character, except the
     * single-octet (ASCII) characters. Throws on errors. Errors
     * include non-minimal encodings.
     */
    class Automaton {
    public:
	explicit Automaton(unsigned char ch) noexcept;
	unsigned add(unsigned char ch) noexcept;
	bool broken = false;
    private:
	unsigned acc;
	unsigned min;
	unsigned char n;
    };


    /**
     * Decode a sequence of unsigned scalars [a, b) as UTF-8 to [out,
     * ...).  Returns b or (on error) the first un-decoded octet; the
     * first octet which hasn't already been part of forming an output
     * character.
     *
     * Detected errors include all non-shortest-form encodings.
     *
     * Narrowing takes place on output.  For example, if you're sure
     * the text is in iso8859-1, you can write into a vector<unsigned
     * char>. An uint32_t is enough for all possible generated
     * characters.
     */
    template <class It, class Out>
    It decode(It a, It b, Out out)
    {
	while(a!=b) {
	    const unsigned char c = *a;

	    if(c & 0x80) {
		auto a0 = a++;
		Automaton au{c};
		if(au.broken) return a0;

		unsigned val = 0;
		while(a!=b) {
		    val = au.add(*a++);
		    if(val) break;
		    if(au.broken) return a0;
		}

		if(val) {
		    *out++ = val;
		}
		else {
		    return a0;
		}
	    }
	    else {
		*out++ = c;
		a++;
	    }
	}
	return a;
    }
}

#endif
