/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <RTMousePField.h>
#include <ugens.h>		// for warn, die


// -------------------------------------------------------- _midi_connection ---
//
//    midi = makeconnection("midi", min, max, default, chan, type, [subtype])
//
//    <type>      "noteon", "noteoff", "cntl", "prog", "bend", "monopress",
//                "polypress"
//
//    <subtype>   depends on <type>:
//                   noteon     "note", "velocity"
//                   noteoff    "note", "velocity"
//                   cntl       controller number or string symbol, such as
//                              "mod", "foot", "breath", "data", "volume", "pan"
//                   prog       not applicable
//                   bend       not applicable
//                   monopress  not applicable
//                   polypress  MIDI note number
//
//



// ------------------------------------------------------- _mouse_connection ---
//
//    mouse = makeconnection("mouseX", min, max, default, prefix, [units,]
//                                                                [precision])
//
//    First argument is either "mouseX" or "mouseY", depending on which axis
//    you want to read.  Other arguements:
//
//    <min>          minimum value [number]
//    <max>          maximum value [number]
//    <default>      value [number]
//    <prefix>       label to display in window [string]
//    <units>        units (e.g., "Hz") to display in window [string]
//    <precision>    digits after decimal point to display in window [number]
//
//                                                               JGG, 8/14/04

static RTNumberPField *
_mouse_usage()
{
	die("makeconnection (mouse)",
		"Usage: makeconnection(\"mouseX\" or \"mouseY\", min, max, default, "
		"prefix, [units,] [precision])");
	return NULL;
}

RTNumberPField *
mouse_connection(const Arg args[], const int nargs)
{
	static RTcmixMouse *mousewin = NULL;
	if (mousewin == NULL)					// first time, so make window
		mousewin = createMouseWindow();

	RTMouseAxis axis = strchr(args[0], 'X') ? RTMouseAxisX : RTMouseAxisY;

	double minval, maxval, defaultval;
	const char *prefix, *units = NULL;
	int precision = 3;

	if (args[1].isType(DoubleType))
		minval = args[1];
	else
		return _mouse_usage();
	if (args[2].isType(DoubleType))
		maxval = args[2];
	else
		return _mouse_usage();
	if (args[3].isType(DoubleType))
		defaultval = args[3];
	else
		return _mouse_usage();
	if (args[4].isType(StringType))
		prefix = args[4];
	else
		return _mouse_usage();
	if (nargs > 5 && args[5].isType(StringType))
		units = args[5];
	else
		return _mouse_usage();
	if (nargs > 6 && args[6].isType(DoubleType))
		precision = (int) args[6];
	else
		return _mouse_usage();

	return new RTMousePField(mousewin, axis, prefix, units,
										precision, minval, maxval, defaultval);
}

