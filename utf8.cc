/* Copyright (c) 2017, 2018 Jörgen Grahn <grahn+src@snipabacken.se>
 * All rights reserved.
 *
 * Collected from utf8(7):
 *
 * [0000 0000 .. 0000 0080)   7  0... ....
 * [0000 0080 .. 0000 0800)  11  110: :::. 10......
 * [0000 0800 .. 0001 0000)  16  1110 :::: 10:..... 10......
 * [0001 0000 .. 0020 0000)  21  1111 0::: 10::.... 10...... 10......
 * [0020 0000 .. 0400 0000)  26  1111 10:: 10:::... 10...... 10...... 10......
 * [0400 0000 .. 8000 0000)  31  1111 110: 10::::.. 10...... 10...... 10...... 10......
 *
 * (: - not all of these bits may be 0)
 *
 */
#include "utf8.h"

namespace {

    unsigned shifted(unsigned n, unsigned ch)
    {
	return ch << (n * 6);
    }
}

using utf8::Automaton;

/**
 * Create the automaton from the first octet, which may not be the
 * single-byte encoding.
 */
Automaton::Automaton(unsigned char ch) noexcept
{
    if((ch & 0xfc) == 0xfc) {
	n = 5;
	acc = shifted(n, ch & 0x03);
	min = 0x4000000;
    }
    else if((ch & 0xf8) == 0xf8) {
	n = 4;
	acc = shifted(n, ch & 0x07);
	min = 0x200000;
    }
    else if((ch & 0xf0) == 0xf0) {
	n = 3;
	acc = shifted(n, ch & 0x0f);
	min = 0x10000;
    }
    else if((ch & 0xe0) == 0xe0) {
	n = 2;
	acc = shifted(n, ch & 0x1f);
	min = 0x800;
    }
    else if((ch & 0xc0) == 0xc0) {
	n = 1;
	acc = shifted(n, ch & 0x3f);
	min = 0x80;
    }
    else {
	broken = true;
    }
}

/**
 * Add an octet to the automaton. Either:
 * - returns 0 because the character isn't complete yet
 * - throws (if it's not one of the 10...... octets, or
 *   the character is complete but not in its minimal encoding)
 * - returns the complete character.
 *
 * Undefined results if you keep using the automaton after
 * it returned its character.
 */
unsigned Automaton::add(unsigned char ch) noexcept
{
    n--;
    if((ch & 0xc0) != 0x80) {
	broken = true;
	return 0;
    }
    acc |= shifted(n, ch & 0x3f);

    if(n) return 0;

    if(acc < min) {
	broken = true;
	return 0;
    }
    return acc;
}
