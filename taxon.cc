/*
 * Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 *
 */
#include "taxon.h"
#include "regex.h"


bool Taxon::match(const Regex& re) const
{
    if(re.match(name)) return true;
    if(re.match(latin)) return true;
    for(std::vector<std::string>::const_iterator i = alias.begin();
	i!=alias.end();
	i++) {
	if(re.match(*i)) return true;
    }
    return false;
}
