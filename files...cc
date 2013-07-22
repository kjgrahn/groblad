/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "files...h"


/**
 * The input position, on the traditional "file:line"
 * format. Standard input is called "<stdin>".
 *
 * Undefined unless the last getline() was successful.
 * Also, invalid if the Files object is destroyed.
 */
Files::Position Files::position() const
{
    static const std::string stdin = "<stdin>";

    return Position(*f=="-" ? stdin : *f, lineno);
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
	fs.open(f->c_str(), std::ios_base::in);
	is = &fs;
    }
    lineno = 1;
}


#if 0
int main(int argc, char** argv)
{
    Files files(&argv[1], &argv[argc]);

    std::string s;
    while(files.getline(s)) {
	std::cout << files.position() << ": " << s << '\n';
    }

    return 0;
}
#endif
