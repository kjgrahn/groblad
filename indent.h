/* -*- c++ -*-
 *
 * Copyright (C) 2013 Jörgen Grahn
 * All rights reserved.
 */
#ifndef GAVIA_INDENT_H
#define GAVIA_INDENT_H

#include <string>
#include <iosfwd>

namespace indent {

    std::ostream& ljust(std::ostream& os, const std::string& s,
			const size_t n);
    std::ostream& rjust(std::ostream& os, const std::string& s,
			const size_t n);
    std::ostream& andjust(std::ostream& os, const std::string& s,
			  const size_t n);
}

#endif
