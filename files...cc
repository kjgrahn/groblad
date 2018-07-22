/*
 * Copyright (c) 2013, 2017, 2018 Jörgen Grahn
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
 */
const Files::Position& Files::position() const
{
    return pos;
}


/**
 * Like position(), but reports on the previous line.  The result is
 * wrong if we're on the first line, but this isn't worth fixing.
 */
Files::Position Files::prev_position() const
{
    Position p{pos};
    p.line--;
    return p;
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
	pos = {std_in, 1};
	is = &std::cin;
    }
    else {
	pos = {*f, 1};
	fs.open(*f, std::ios_base::in);
	is = &fs;
	if(!fs.is_open()) {
	    std::cerr << "error: cannot open '" << pos.file
		      << "' for reading: " << std::strerror(errno) << '\n';
	}
    }
}
