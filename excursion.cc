/* Copyright (c) 1999, 2008, 2011, 2013 J�rgen Grahn
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

#include "lineparse.h"
#include "files...h"

#include <algorithm>
#include <tr1/unordered_set>


namespace {

    bool known_header(const std::string& name)
    {
	const std::string known[] = {
	    "place",
	    "coordinate",
	    "date",
	    "observers",
	    "comments",
	    "status",
	};
	const std::string* const end = known + sizeof known/sizeof known[0];
	return std::find(known, end, name) != end;
    }
}


void Excursion::swap(Excursion& other)
{
    headers.swap(other.headers);
    sightings.swap(other.sightings);
    std::swap(date, other.date);
    std::swap(place, other.place);
}


/**
 * Add a header. Returns false if the header name isn't familiar
 * (but adds it anyway).
 */
bool Excursion::add_header(const char* a, size_t alen,
			   const char* b, size_t blen)
{
    const Header h(std::string(a, alen),
		   std::string(b, blen));
    headers.push_back(h);
    return known_header(h.name);
}


/**
 * Add a line text to the last header value. Returns false and does
 * nothing if there is no such header.
 */
bool Excursion::add_header_cont(const char* a, size_t alen)
{
    if(headers.empty()) return false;
    Header& h = headers.back();
    h.value.append("\n");
    h.value.append(a, alen);
    return true;
}


/**
 * Add a sighting. Returns false if the taxon isn't familiar
 * (but adds it anyway, and invents a taxon on the fly).
 */
bool Excursion::add_sighting(Taxa& spp,
			     const char* a, size_t alen,
			     const char* b, size_t blen)
{
    const std::string name(a, alen);
    TaxonId id = spp.find(name);
    const bool familiar = id;
    if(!familiar) {
	id = spp.insert(name);
    }

    sightings.push_back(Sighting(id, name,
				 std::string(b, blen)));
    return familiar;
}


bool Excursion::add_sighting_cont(const char* a, size_t alen)
{
    if(sightings.empty()) return false;
    Sighting& s = sightings.back();
    s.comment.append("\n");
    s.comment.append(a, alen);
    return true;
}


/**
 * Perform last fixups, after the last add_sighting() etc.
 * Does things like parse the date: header for faster later use.
 */
bool Excursion::finalize()
{
    const std::string& s = find_header("date");
    const char* a = s.c_str();
    date = Date(a, a+s.size());
    place = find_header("place");
    return true;
}


namespace {

    struct IsTaxon {
	bool operator() (const Excursion::Sighting& s, TaxonId sp) {
	    return s.sp == sp;
	}
    };
}


/**
 * True if at least one of 'taxa' is present.
 */
bool Excursion::has_one(const std::vector<TaxonId>& taxa) const
{
    return std::find_first_of(sightings.begin(), sightings.end(),
			      taxa.begin(), taxa.end(),
			      IsTaxon()) != sightings.end();
}


namespace {

    template<class T>
    struct IsNamed {
	explicit IsNamed(const char* name) : name(name) {}
	const char* const name;
	bool operator() (const T& t) { return t.name==name; }
    };
}


/**
 * Find the first header named 'name' and return its value, or "".
 * Returns by reference, so you'd better not modify the headers afterwards.
 */
const std::string& Excursion::find_header(const char* name) const
{
    static const std::string NIL;
    Headers::const_iterator i = std::find_if(headers.begin(),
					     headers.end(),
					     IsNamed<Header>(name));
    if(i==headers.end()) return NIL;
    return i->value;
}


namespace {

    struct Errlog {
	Errlog(std::ostream& errstream,
	       const Files& files)
	    : errstream(errstream),
	      files(files)
	{}

	void general(const std::string& s);
	void header(const std::string& s) { general(s); }
	void sighting(const std::string& s) { general(s); }
	void trailer();

	void warn_header(const char* s, size_t len);
	void warn_sighting(const char* s, size_t len);

	std::ostream& errstream;
	const Files& files;
    };

    void Errlog::general(const std::string& s)
    {
	errstream << files.position() << ": parse error: " << s << '\n';
    }

    void Errlog::trailer()
    {
	errstream << files.position() << ": trailing partial excursion ignored\n";
    }

    void Errlog::warn_header(const char* s, size_t len)
    {
	errstream << files.position() << ": unfamiliar header field \"";
	errstream.write(s, len) << "\"\n";
    }

    void Errlog::warn_sighting(const char* s, size_t len)
    {
	errstream << files.position() << ": unfamiliar taxon \"";
	errstream.write(s, len) << "\"\n";
    }
}


/**
 * Read one excursion from 'is', using and possibly augmenting 'spp'
 * meanwhile.  Logs errors (warnings, really) to 'err'.
 * Returns false at eof with no complete excursion read.
 */
bool get(Files& is, std::ostream& errstream,
	 Taxa& spp, Excursion& excursion)
{
    using Parse::ws;
    using Parse::trimr;

    Errlog err(errstream, is);

    enum State { BETWEEN, HEADERS, SIGHTINGS };
    State state = BETWEEN;
    Excursion ex;
    std::string s;

    while(is.getline(s)) {

	const char* a = s.c_str();
	const char* const b = trimr(a, a + s.size());

	const char* c = ws(a, b);
	if(c==b || *c=='#') continue;

	if(state==BETWEEN) {

	    if(*a=='{' && a+1==b) {
		state = HEADERS;
	    }
	    else {
		err.general(s);
		continue;
	    }
	}
	else if(state==HEADERS && c==a) {

	    if(a+2==b && a[0]=='}' && a[1]=='{') {
		state = SIGHTINGS;
		continue;
	    }

	    c = std::find(a, b, ':');
	    if(c==b) {
		err.header(s);
		continue;
	    }

	    const char* d = trimr(a, c);
	    c = ws(c+1, b);

	    /* [a, d) : [c, b) */
	    if(!ex.add_header(a, d-a, c, b-c)) {
		err.warn_header(a, d-a);
	    }
	}
	else if(state==HEADERS) {

	    /* continuation ___ [c, b) */
	    if(!ex.add_header_cont(c, b-c)) {
		err.header(s);
	    }
	}
	else if(state==SIGHTINGS && c==a) {

	    if(a+1==b && *a=='}') {
		ex.finalize();
		ex.swap(excursion);
		return true;
	    }

	    /* species : marker : comment
	     * a       c        d        b
	     */
	    c = std::find(a, b, ':');
	    if(c==b) {
		err.sighting(s);
		continue;
	    }
	    const char* d = std::find(c+1, b, ':');
	    if(d==b) {
		err.sighting(s);
		continue;
	    }

	    /* species : marker : comment
	     * a      .  c     .  d      b
	     */
	    const char* ae = trimr(a, c);
	    c = ws(c+1, b);
	    const char* ce = trimr(c, d);
	    d = ws(d+1, b);
	    const char* de = trimr(d, b);
	    if(a==ae) {
		err.sighting(s);
		continue;
	    }
	    if(c==ce && d==de) {
		/* unfilled */
		continue;
	    }

	    /* [a, ae) : marker : [d, de) */
	    if(!ex.add_sighting(spp,
				a, ae-a,
				d, de-d)) {
		err.warn_sighting(a, ae-a);
	    }
	}
	else if(state==SIGHTINGS) {

	    /* continuation ___ [c, b) */
	    if(!ex.add_sighting_cont(c, b-c)) {
		err.sighting(s);
	    }
	}
    }

    if(state!=BETWEEN) {
	err.trailer();
    }
    return false;
}
