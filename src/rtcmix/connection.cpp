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
#include <rtcmix_types.h>
#include <PField.h>
#include <ugens.h>		// for warn, die

extern RTNumberPField *mouse_connection(const Arg args[], const int nargs);
extern RTNumberPField *midi_connection(const Arg args[], const int nargs);


// --------------------------------------------------------- local utilities ---
static Handle
_createPFieldHandle(PField *pfield)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	handle->type = PFieldType;
	handle->ptr = (void *) pfield;
	return handle;
}


// =============================================================================
// The remaining functions are public, callable from scripts.

extern "C" {
	Handle makeconnection(const Arg args[], const int nargs);
};


// ---------------------------------------------------------- makeconnection ---
Handle
makeconnection(const Arg args[], const int nargs)
{
	if (!args[0].isType(StringType)) {
		die("makeconnection", "First argument must be a string giving "
									"connection type, e.g. \"mouseX\", \"midi\".");
		return NULL;
	}

	RTNumberPField *connection = NULL;

	if (args[0] == "mouseX" || args[0] == "mouseY")
#ifdef MACOSX
{
		die("makeconnection (mouse)", "Not yet implemented for OS X.");
		return NULL;
}
#else
		connection = mouse_connection(args, nargs);
#endif

	if (connection == NULL)
		return NULL;

	return _createPFieldHandle(connection);
}

