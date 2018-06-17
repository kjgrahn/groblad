/* -*- c++ -*-
 *
 * Copyright (C) 2013 Jörgen Grahn
 * All rights reserved.
 */
#ifndef GAVIA_INDENT_H
#define GAVIA_INDENT_H

#include <string>
#include <iosfwd>

/**
 * Measuring text width, and indenting based on text width.
 *
 * Can measure UTF-8 text width (in Unicode characters) but falls back
 * to string::size() on UTF-8 encoding errors -- thus it works on
 * iso8859-1 text too.  Once it has decided it's not UTF-8, it stops
 * trying that strategy.
 */
class Indent {
public:
    size_t measure(const std::string& s);

    std::ostream& ljust(std::ostream& os, const std::string& s,
			const size_t n);
    std::ostream& rjust(std::ostream& os, const std::string& s,
			const size_t n);

    std::ostream& andjust(std::ostream& os, const std::string& s,
			  const size_t n) const;

private:
    bool utf8 = true;
};

#endif
