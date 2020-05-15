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

#include "rtcmix_types.h"
#include <PField.h>
#include <ugens.h>		// for warn, die
#include <Option.h>

#include "DynamicLib.h"

#if !defined(EMBEDDED) && !defined(SHAREDLIBDIR)
#error "Compile flags are missing macro for SHAREDLIBDIR (IGNORE THIS FOR 'make depend')"
#endif

// =============================================================================
// The remaining functions are public, callable from scripts.

extern "C" {
	Handle makeconnection(const Arg args[], const int nargs);
#ifdef EMBEDDED
// BGG -- see note below
	Handle create_handle(const Arg args[], const int nargs);
	Handle create_pfbus_handle(const Arg args[], const int nargs);
#endif
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
#ifndef EMBEDDED
	const char *selector = (const char *) args[0];
	char loadPath[1024];
	const char *dsoPath = Option::dsoPath();
	if (strlen(dsoPath) == 0)
		dsoPath = SHAREDLIBDIR;
	sprintf(loadPath, "%s/lib%sconn.so", dsoPath, selector);

	DynamicLib theDSO;
	if (theDSO.load(loadPath) == 0) {
		HandleCreator creator = NULL;
		if (theDSO.loadFunction(&creator, "create_handle") == 0) {
			// Pass 2nd thru last args, leaving off selector
			handle = (*creator)(&args[1], nargs - 1);
		}
		else {
			die("makeconnection", "symbol lookup failed: %s\n", theDSO.error());
			theDSO.unload();
			return NULL;
		}
	}
	else {
		die("makeconnection", "dso load failed: %s\n", theDSO.error());
		return NULL;
	}

#else 
// BGG -- create_handle() will invoke the RTInletPField stuff in control/maxmsp
// We can't use the 'normal' create_pfield() because of conflict with
// inletglue.cpp function (dyn loading keeps them separate, but we
// don't dynlaod in max/msp)
// same with create_pfbus_handle()

	if (args[0] == "inlet") {
		handle = create_handle(&args[1], nargs-1);
	} else if(args[0] == "pfbus") {
		handle = create_pfbus_handle(&args[1], nargs-1);
	}
#endif // EMBEDDED

	return handle;
}

