// RTcmix  - Copyright (C) 2005  The RTcmix Development Team
// See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
// the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

// glue.cpp - user interface for OSC server, by John Gibson

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <rtcmix_types.h>
#include <Option.h>
#include <PField.h>
#include <utils.h>	// in ../../rtcmix
#include <RTcmixOSC.h>
#include <RTOscPField.h>
#include <ugens.h>		// for warn, die

// -----------------------------------------------------------------------------
//
//   osc = makeconnection("osc", path, index, min, max, default, lag
//                          [, input_min, input_max])
//
//   <path>      OSC path string to match (e.g., "/synth/oscil2/frequency")
//
//   <index>     zero-based index of item in message to read
//
//   <min, max>  range of numbers to output
//
//   <default>   number to output before first matching OSC message received
//
//   <lag>       amount of smoothing of output signal (0-100)
//
//   <input_min, range of raw data to expect.  If missing, expect [0-1].
//   input_max>
//
//
//   Example:
//
//   osc = makeconnection("osc", "/synth/freqs", index=1, min=100,
//                         max=1000, default=100, lag=40)
//
//   If the OSC server receives messages with a path of "/synth/freqs",
//   this PField returns the second item as a float, scaled to fit between
//   100 and 1000 inclusive.  Incoming OSC values are assumed to fall between
//   0 and 1.  The PField returns 100 if accessed prior to receiving any
//   matching OSC messages.  The stream of values is smoothed using a lag
//   value of 40.

static RTNumberPField *
_osc_usage()
{
	die("makeconnection (osc)",
		"Usage: makeconnection(\"osc\", path, index, min, max, default, "
		"lag[, input_min, input_max])");
	return NULL;
}

static RTNumberPField *
create_pfield(const Arg args[], const int nargs)
{
	if (nargs < 6)
		return _osc_usage();

	double outputmin, outputmax, defaultval, lag, inputmin = 0.0, inputmax = 1.0;
	int index;
	const char *path;

	if (args[0].isType(StringType))
		path = args[0];
	else
		return _osc_usage();

	if (args[1].isType(DoubleType))
		index = (int) args[1];
	else
		return _osc_usage();

	if (args[2].isType(DoubleType))
		outputmin = args[2];
	else
		return _osc_usage();

	if (args[3].isType(DoubleType))
		outputmax = args[3];
	else
		return _osc_usage();

	if (args[4].isType(DoubleType))
		defaultval = args[4];
	else
		return _osc_usage();

	if (args[5].isType(DoubleType))
		lag = args[5];
	else
		return _osc_usage();
	if (lag < 0.0 || lag > 100.0) {
		die("makeconnection (osc)", "<lag> must be between 0 and 100");
		return NULL;
	}
	lag *= 0.01;

	if (nargs > 6) {
		if (args[6].isType(DoubleType))
			inputmin = args[6];
		else
			return _osc_usage();
		if (nargs > 7) {
			if (args[7].isType(DoubleType))
				inputmax = args[7];
			else
				return _osc_usage();
		}
		else
			return _osc_usage();
	}

	static RTcmixOSC *oscserver = NULL;
	if (oscserver == NULL)				// first time, so init OSC server
		oscserver = createOSCServer();
	if (oscserver == NULL)
		return NULL;

	return new RTOscPField(oscserver, path, index, inputmin, inputmax,
	                           outputmin, outputmax, defaultval, lag);
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
		handle = createPFieldHandle(pField);
	}
	return handle;
}

int register_dso()
{
	 return 0;
}
