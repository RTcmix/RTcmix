/* RTcmix  - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <DisplayPField.h>
#include <RTcmixDisplay.h>
#include <ugens.h>		// for warn, die


// ------------------------------------------------------------- makedisplay ---
//
//    display = makedisplay(handle, [prefix[, units,]] [precision])
//
//    First argument is a PField handle, such as that returned by a call
//    to makeconnection("midi", ...). Other arguments, which are optional:
//
//    <prefix>       label to display in window [string]
//    <units>        units (e.g., "Hz") to display in window [string]
//    <precision>    digits after decimal point to display in window [number]
//
//    If <prefix> is missing or an empty string, then no label printed in
//    window.
//                                                               JGG, 2/8/05

static PFieldWrapper *
_makedisplay_usage()
{
	die("makedisplay",
		"Usage: makedisplay(handle, [prefix[, units,]] [precision])");
	return NULL;
}

static PFieldWrapper *
create_pfield(const Arg args[], const int nargs)
{
	if (nargs < 1)
		return _makedisplay_usage();

	if (!args[0].isType(HandleType)) {
		die("makedisplay", "First argument must be a valid pfield handle.");
		return NULL;
	}
	PField *pfield = (PField *) args[0];

	const char *prefix = NULL, *units = NULL;
	int precision = 3;

	// Handle the optional arguments.
	// <prefix> and <units> strings must appear in that order.  <precision>
	// follows either or both of the strings, or replaces both.  (I.e., it
	// appears after of a list of 0, 1, or 2 strings.)
	if (nargs > 1) {
		if (args[1].isType(StringType)) {
			prefix = args[1];
			if (nargs > 2) {
				if (args[2].isType(StringType)) {
					units = args[2];
					if (nargs > 3) {
						if (args[3].isType(DoubleType))
							precision = (int) args[3];
						else
							return _makedisplay_usage();
					}
				}
				else
					precision = (int) args[2];
			}
		}
		else
			precision = (int) args[1];
	}

	static RTcmixDisplay *displaywin = NULL;
	if (displaywin == NULL)					// first time, so make window
		displaywin = createDisplayWindow();
	if (displaywin == NULL) {
		die("makedisplay", "Failed to create display window");
		return NULL;
	}

	return new DisplayPField(pfield, displaywin, prefix, units, precision);
}

// The following functions are the publicly-visible ones called by the
// system.

extern "C" {
	Handle create_handle(const Arg args[], const int nargs);
	int register_dso();
};

Handle
create_handle(const Arg args[], const int nargs)
{
	PField *pField = create_pfield(args, nargs);
	Handle handle = NULL;
	if (pField != NULL) {
		handle = new struct _handle;
		handle->type = PFieldType;
		handle->ptr = (void *) pField;
	}
	return handle;
}

int register_dso()
{
	 return 0;
}

