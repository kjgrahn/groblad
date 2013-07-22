/* -*- c++ -*-
 *
 * Copyright (C) 2001, 2013 Jörgen Grahn
 * All rights reserved.
 */
#ifndef GAVIA_REGEX_H
#define GAVIA_REGEX_H

#include <string>
#include <cassert>

/**
 * A simple wrapper around the basic parts of
 * POSIX Extended Regular Expressions.
 *
 */
class Regex
{
public:
    explicit Regex(const std::string& regex);
    ~Regex();

    bool bad() const;
    std::string error() const;

    bool match(const std::string& s) const { return match(s.c_str()); }
    bool match(const char* s) const;

    struct Wrapper;

private:
    Regex();
    Regex(const Regex&);

    Wrapper* const wrapper;
};

#endif
