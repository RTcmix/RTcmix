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
#include <RTcmixMouse.h>
#include <ugens.h>		// for warn, die


// ------------------------------------------------------- _mouse_connection ---
//
//    mouse = makeconnection("mouseX", min, max, default, lag,
//                                     [prefix[, units[, precision]]])
//
//    First argument is either "mouseX" or "mouseY", depending on which axis
//    you want to read.  Other arguements:
//
//    <min>          minimum value [number]
//    <max>          maximum value [number]
//    <default>      value returned before mouse first enters window [number]
//    <lag>          amount of smoothing for value stream [percent: 0-100]
//    <prefix>       label to display in window [string]
//    <units>        units (e.g., "Hz") to display in window [string]
//    <precision>    digits after decimal point to display in window [number]
//
//    Making <min> greater than <max> lets you invert values.
//
//    If <prefix> is missing or an empty string, then no label printed in
//    mouse window.
//                                                               JGG, 8/14/04

static RTNumberPField *
_mouse_usage()
{
	die("makeconnection (mouse)",
		"Usage: makeconnection(\"mouseX\" or \"mouseY\", min, max, default, "
		"lag, [prefix[, units[, precision]]])");
	return NULL;
}

RTNumberPField *
mouse_connection(const Arg args[], const int nargs)
{
	static RTcmixMouse *mousewin = NULL;
	if (mousewin == NULL)					// first time, so make window
		mousewin = createMouseWindow();

	RTMouseAxis axis = strchr(args[0], 'X') ? kRTMouseAxisX : kRTMouseAxisY;

	double minval, maxval, defaultval, lag;
	const char *prefix = NULL, *units = NULL;
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

	if (args[4].isType(DoubleType))
		lag = args[4];
	else
		return _mouse_usage();
	if (lag < 0.0 || lag > 100.0) {
		die("makeconnection (mouse)", "<lag> must be between 0 and 100");
		return NULL;
	}

	if (nargs > 5) {
		if (args[5].isType(StringType))
			prefix = args[5];
		else
			return _mouse_usage();
	}

	if (nargs > 6) {
		if (args[6].isType(StringType))
			units = args[6];
		else
			return _mouse_usage();
	}

	if (nargs > 7) {
		if (args[7].isType(DoubleType))
			precision = (int) args[7];
		else
			return _mouse_usage();
	}

	return new RTMousePField(mousewin, axis, minval, maxval, defaultval, lag,
										prefix, units, precision);
}

