/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Functions for creating and modifying control streams.  These can be
   passed from a script to RTcmix functions that accept them.  See also
   table.cpp for a similar set of functions.

   John Gibson, 8/14/04
*/
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include <rtcmix_types.h>
#include <PField.h>
#include <ugens.h>		// for warn, die

#if !defined(SHAREDLIBDIR) || !defined(SHLIB_SUFFIX)
#error "Compile flags are missing macros for SHAREDLIBDIR and/or SHLIB_SUFFIX"
#endif

// --------------------------------------------------------- local utilities ---
void *
find_dso(const char *selector)
{
	char loadPath[1024];
	dlerror();	// clear error queue
	sprintf(loadPath, "%s/lib%sconn.%s", SHAREDLIBDIR, selector, SHLIB_SUFFIX);
	void *dso = dlopen(loadPath, RTLD_LAZY);
	if (!dso) {
		die("makeconnection", "dynamic load failed: %s\n", dlerror());
	}
	return dso;
}

// =============================================================================
// The remaining functions are public, callable from scripts.

extern "C" {
	Handle makeconnection(const Arg args[], const int nargs);
};

typedef Handle (*HandleCreator)(const Arg[], const int);

// ---------------------------------------------------------- makeconnection ---
Handle
makeconnection(const Arg args[], const int nargs)
{
	if (!args[0].isType(StringType)) {
		die("makeconnection", "First argument must be a string giving "
			"connection type, e.g. \"mouse\", \"midi\".");
		return NULL;
	}

	if (args[0] == "mouseX" || args[0] == "mouseY") {
		die("makeconnection",
			"New calling convention for mouse is (\"mouse\", \"X\", ...)");
		return NULL;
	}

	Handle handle = NULL;
	HandleCreator creator = NULL;
	const char *selector = (const char *) args[0];

	void *dso = find_dso(selector);
	if (dso) {
		creator = (HandleCreator) dlsym(dso, "create_handle");
		if (creator) {
			// Pass 2nd thru last args, leaving off selector
			handle = (*creator)(&args[1], nargs - 1);
		}
		else {
			die("makeconnection", "symbol lookup failed: %s\n", dlerror());
		}
	}
	return handle;
}

