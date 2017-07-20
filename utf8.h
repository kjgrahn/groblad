/* -*- c++ -*-
 *
 * Copyright (c) 2017 Jörgen Grahn <grahn+src@snipabacken.se>
 * All rights reserved.
 */
#ifndef GAVIA_UTF8_H
#define GAVIA_UTF8_H

namespace utf8 {

    class DecodeError {};

    /**
     * Automaton for decoding of an UTF-8 character, except the
     * single-octet (ASCII) characters. Throws on errors. Errors
     * include non-minimal encodings.
     */
    class Automaton {
    public:
	explicit Automaton(unsigned char ch);
	unsigned add(unsigned char ch);
    private:
	unsigned acc;
	unsigned min;
	unsigned n;
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

	    if (!(c & 0x80)) {
		*out++ = c;
		a++;
		continue;
	    }

	    try {
		Automaton au(c);

		It i = a;
		i++;

		while(i!=b) {
		    unsigned val = au.add(*i++);
		    if (val) {
			*out++ = val;
			a = i;
			break;
		    }
		}
	    }
	    catch (DecodeError) {
		break;
	    }
	}
	return a;
    }
}

#endif
