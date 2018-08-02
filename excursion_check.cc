/* Copyright (c) 2013 Jörgen Grahn
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "excursion.h"
#include "taxa.h"
#include "files...h"
#include "date.h"
#include "coordinate.h"

#include <algorithm>
#include <iostream>


/**
 * Emit warnings about 'excursion'.
 * These are warnings which aren't high-level parse errors, but relate
 * to one header or sighting, or to the headers or sightings as a
 * whole.
 *
 * The funny interface exists basically so that we can provide
 * relevant line numbers, by doing the check as early as possible.
 */

namespace {

    struct taxon_of {
	TaxonId operator() (const Excursion::Sighting& val) const {
	    return val.sp;
	}
    };
}


void check_sightings(const Excursion& ex, Taxa& taxa,
		     const Files& is, std::ostream& err)
{
    std::vector<TaxonId> spp(std::distance(ex.sbegin(), ex.send()));
    std::transform(ex.sbegin(), ex.send(), begin(spp), taxon_of());

    const std::vector<TaxonId>::const_iterator end = spp.end();
    std::vector<TaxonId>::const_iterator i = begin(spp);
    while((i = std::adjacent_find(i, end)) != end) {
	err << is.position() << ": duplicate entries for taxon \""
	    << taxa[*i].name << "\"\n";
	i++;
    }
}


void check_last_header(const Excursion& ex,
		       const Files& is, std::ostream& err)
{
    if(ex.hbegin()==ex.hend()) return;
    const Excursion::Header& h = *(ex.hend()-1);

    if(h.empty()) return;

    /* A few headers aren't really free-form. */
    if(h.name == "coordinate") {
	const char* const s = h.value.c_str();
	const Coordinate coord(s, s + h.value.size());
	if(!coord.valid()) {
	    err << is.prev_position()
		<< ": malformed coordinate \"" << h.value << "\"\n";
	}
    }
    else if(h.name == "date") {
	const char* const s = h.value.c_str();
	const Date date(s, s + h.value.size());
	if(!date.valid()) {
	    err << is.prev_position()
		<< ": malformed date \"" << h.value << "\"\n";
	}
    }
}


void check_last_sighting(const Excursion&,
			 const Files&, std::ostream&)
{
    /* Nothing to check, really.  The validity of names has already
     * been checked, and there's no point in restricting the comment.
     */
}
