/*
 * Copyright (c) 2013, 2017 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "files...h"

#include <cstring>

namespace {
    static const std::string std_in = "<stdin>";
}


/**
 * The input position, on the traditional "file:line"
 * format. Standard input is called "<stdin>".
 *
 * Undefined unless the last getline() was successful.
 * Also, invalid if the Files object is destroyed.
 */
Files::Position Files::position() const
{
    return Position(*f=="-" ? std_in : *f, lineno);
}


/**
 * Like position(), but reports on the previous line.  The result is
 * wrong if we're on the first line, but this isn't worth fixing.
 */
Files::Position Files::prev_position() const
{
    return Position(*f=="-" ? std_in : *f, lineno-1);
}


std::ostream& operator<< (std::ostream& os, const Files::Position& val)
{
    return os << val.file << ':' << val.line;
}


/**
 * The slowpath part of getline(). Called whenever an ordinary
 * std::getline() fails.
 */
bool Files::getline_helper(std::string& s)
{
    if(ff.empty()) return false;

    if(!is && f==ff.begin()) {
	/* first getline() ever */
	open();
    }

    while(!std::getline(*is, s)) {

	fs.close();
	f++;
	if(f==ff.end()) {
	    return false;
	}
	else {
	    open();
	}
    }

    return true;
}


void Files::open()
{
    if(*f=="-") {
	is = &std::cin;
    }
    else {
	fs.open(*f, std::ios_base::in);
	is = &fs;
	if(!fs.is_open()) {
	    std::cerr << "error: cannot open '" << *f
		      << "' for reading: " << std::strerror(errno) << '\n';
	}
    }
    lineno = 1;
}
