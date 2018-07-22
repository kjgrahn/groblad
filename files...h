/* -*- c++ -*-
 *
 * Copyright (c) 2012, 2013, 2018 Jörgen Grahn
 * All rights reserved.
 */
#ifndef FILES___H
#define FILES___H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>


/**
 * An implementation of the most important parts of Perl's
 * 'while(<>) ...'
 * or the traditional Unix command-line convention
 * 'foo [options] file ...'
 *
 * In other words, given a list of file names, support for reading
 * them line by line as if they are pretty much one long file. Not
 * too hard to reimplement for each tool, except:
 * - you want "-" to mean standard input
 * - you probably want an empty list to mean standard input, too
 * - you want file name and line number information for diagnostics,
 *   even for standard input
 *
 */
class Files {
public:
    template <class It>
    Files(It begin, It end,
	  bool empty_is_stdin = true);

    bool getline(std::string& s);

    struct Position {
	Position(const std::string& file, const unsigned line)
	    : file(file),
	      line(line)
	{}
	std::string file;
	unsigned line;
    };
    const Position& position() const;
    Position prev_position() const;

private:
    Files(const Files&);
    Files& operator= (const Files&);

    bool getline_helper(std::string& s);
    void open();

    std::vector<std::string> ff;
    std::vector<std::string>::const_iterator f;
    std::istream* is;
    std::ifstream fs;
    Position pos;
};


/**
 * Like std::getline(string&), but operates on the sequence of lines
 * formed by all involved files.  When it returns success, a line is
 * filled into 's' and position() is correct.
 */
inline
bool Files::getline(std::string& s)
{
    pos.line++;
    if(is && std::getline(*is, s)) {
	return true;
    }

    return getline_helper(s);
}


/**
 * Read from the file names in [begin .. end). "-" means standard
 * input, and so does an empty range, unless empty_is_stdin is set.
 */
template <class It>
Files::Files(It begin, It end,
	     bool empty_is_stdin)
    : ff(begin, end),
      is(0),
      pos{"", 0}
{
    if(ff.empty() && empty_is_stdin) ff.push_back("-");
    f = ff.begin();
}


std::ostream& operator<< (std::ostream& os, const Files::Position& val);

#endif
