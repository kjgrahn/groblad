/* Copyright (c) 2017 Jörgen Grahn
 * All rights reserved.
 *
 */
#include <utf8.h>

#include <list>
#include <string>
#include <vector>

#include <orchis.h>

namespace encoding {

    void ascii(orchis::TC)
    {
	const std::string src{"foo"};
	std::string dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_eq(src, dst);
	orchis::assert_true(it == end(src));
    }

    void bits11(orchis::TC)
    {
	std::vector<unsigned char> src{0xc2, 0x80,
				       0xdf, 0xbf};
	std::vector<unsigned> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0],  0x80);
	orchis::assert_eq(dst[1], 0x7ff);
    }

    void bits16(orchis::TC)
    {
	std::vector<unsigned char> src{0xe0, 0xa0, 0x80,
				       0xef, 0xbf, 0xbf};
	std::vector<unsigned> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0],  0x800);
	orchis::assert_eq(dst[1], 0xffff);
    }

    void bits21(orchis::TC)
    {
	std::vector<unsigned char> src{0xf0, 0x90, 0x80, 0x80,
				       0xf7, 0xbf, 0xbf, 0xbf};
	std::vector<unsigned> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0],  0x10000);
	orchis::assert_eq(dst[1], 0x1fffff);
    }

    void bits26(orchis::TC)
    {
	std::vector<unsigned char> src{0xf8, 0x88, 0x80, 0x80, 0x80,
				       0xfb, 0xbf, 0xbf, 0xbf, 0xbf};
	std::vector<unsigned> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0],  0x200000);
	orchis::assert_eq(dst[1], 0x3ffffff);
    }

    void bits31(orchis::TC)
    {
	std::vector<unsigned char> src{0xfc, 0x84, 0x80, 0x80, 0x80, 0x80,
				       0xfd, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf};
	std::vector<unsigned> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0],  0x4000000);
	orchis::assert_eq(dst[1], 0x7fffffff);
    }

    void narrowing(orchis::TC)
    {
	std::vector<unsigned char> src{0xfc, 0x84, 0x80, 0x80, 0x80, 0x80,
				       0xfd, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf};
	std::vector<unsigned char> dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_true(it == end(src));
	orchis::assert_eq(dst.size(), 2);
	orchis::assert_eq(dst[0], 0x00);
	orchis::assert_eq(dst[1], 0xff);
    }

    void list(orchis::TC)
    {
	const std::string src0{"foo"};
	const std::list<char> src{begin(src0), end(src0)};
	std::string dst;
	auto it = utf8::decode(begin(src), end(src),
			       std::back_inserter(dst));
	orchis::assert_eq(src0, dst);
	orchis::assert_true(it == end(src));
    }

    namespace bad {

	// No part of 'src' decodes to anything.
	template <class Cont>
	void assert_nothing(const Cont& src)
	{
	    std::string dst;
	    auto it = utf8::decode(begin(src), end(src),
				   std::back_inserter(dst));
	    orchis::assert_eq(dst, "");
	    orchis::assert_true(it == begin(src));
	}

	// The first two octets of 'src' decode to a NBSP.
	template <class Cont>
	void assert_nbsp(const Cont& src)
	{
	    std::string dst;
	    auto it = utf8::decode(begin(src), end(src),
				   std::back_inserter(dst));
	    orchis::assert_eq(dst, "\xa0");
	    orchis::assert_true(it == begin(src) + 2);
	}

	void latin1(orchis::TC)
	{
	    const std::string src{"J\xf6G"};
	    std::string dst;
	    auto it = utf8::decode(begin(src), end(src),
				   std::back_inserter(dst));
	    orchis::assert_eq("J", dst);
	    orchis::assert_true(it == begin(src) + 1);
	}

	void latin1_trailer(orchis::TC)
	{
	    const std::string src{"J\xf6"};
	    std::string dst;
	    auto it = utf8::decode(begin(src), end(src),
				   std::back_inserter(dst));
	    orchis::assert_eq("J", dst);
	    orchis::assert_true(it == begin(src) + 1);
	}

	void not_minimal(orchis::TC)
	{
	    const unsigned val = '0' | 0x80;
	    std::vector<unsigned> src11{0xc0, val};
	    std::vector<unsigned> src16{0xe0, 0x80, val};
	    std::vector<unsigned> src21{0xf0, 0x80, 0x80, val};
	    std::vector<unsigned> src26{0xf8, 0x80, 0x80, 0x80, val};
	    std::vector<unsigned> src31{0xfc, 0x80, 0x80, 0x80, 0x80, val};
	    assert_nothing(src11);
	    assert_nothing(src16);
	    assert_nothing(src21);
	    assert_nothing(src26);
	    assert_nothing(src31);
	}

	void trailing_partial(orchis::TC)
	{
	    using src = std::vector<unsigned>;
	    assert_nbsp(src{0xc2, 0xa0});
	    assert_nbsp(src{0xc2, 0xa0,  0xcf});
	    assert_nbsp(src{0xc2, 0xa0,  0xef, 0x80});
	    assert_nbsp(src{0xc2, 0xa0,  0xf7, 0x80, 0x80});
	    assert_nbsp(src{0xc2, 0xa0,  0xfb, 0x80, 0x80, 0x80});
	    assert_nbsp(src{0xc2, 0xa0,  0xfd, 0x80, 0x80, 0x80, 0x80});
	}
    }
}
