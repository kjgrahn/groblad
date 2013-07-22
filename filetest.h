/* -*- c++ -*-
 *
 * Copyright (C) 2013 Jörgen Grahn
 * All rights reserved.
 */
#ifndef GAVIA_FILETEST_H
#define GAVIA_FILETEST_H
#include <string>

namespace filetest {

    bool writeable(const std::string& f);
    bool readable(const std::string& f);
}

#endif
