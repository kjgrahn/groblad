/* Copyright (c) 2013, 2018 Jörgen Grahn
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
#include "editor.h"

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


namespace {

    std::string editor_name()
    {
	const char* p = 0;
	if(!p) p = std::getenv("GAVIAEDITOR");
	if(!p) p = std::getenv("VISUAL");
	if(!p) p = std::getenv("EDITOR");
	if(!p) p = "emacs";
	return p;
    }
}


/**
 * Like system("emacs $path"), but tries a few editors, doesn't use
 * the shell, and returns true iff it returns normally with a zero
 * exit status.
 */
bool editor(const std::string& path,
	    const std::string& reference)
{
    const std::string ed = editor_name();

    const pid_t pid = fork();
    if(pid==-1) return false;

    if(pid) {
	int rc;
	while(waitpid(pid, &rc, 0)==-1 && errno==EINTR) {}
	return WIFEXITED(rc) && WEXITSTATUS(rc)==0;
    }
    else {
	static const char* null;
	if(ed=="emacs") {
	    execlp(ed.c_str(), ed.c_str(),
		   "+2:100",
		   path.c_str(),
		   reference.c_str(),
		   "-f",
		   "delete-window",
		   null);
	}
	else {
	    execlp(ed.c_str(), ed.c_str(), path.c_str(), null);
	}
	std::exit(1);
    }
}
