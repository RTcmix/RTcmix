/* RTcmix  - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <DataFileReaderPField.h>
#include <ugens.h>		// for warn, die
#include <utils.h>

extern int resetval;		// declared in src/rtcmix/minc_functions.c


// -----------------------------------------------------------------------------
//
//    stream = makeconnection("datafile", filename, lag,
//                            [timefactor[, filerate, format, swap]])
//
//    <filename>     full or relative path to data file [string]
//    <lag>          amount of smoothing for value stream [percent: 0-100]
//
//    The next argument is optional.
//
//    <timefactor>   scale time it takes to consume file [1: use the same
//                   amount of time it took to create the file; 2: take twice
//                   as long to play the file data; 0.5: take half as long;
//                   default is 1.0]
//
//    The next three are optional, used only if the data file has no header.
//    If you give any, you must give all three.  We recommend that you give a
//    filerate that's significantly less than the current control rate (maybe
//    1/5 as fast), so as to thin the file data a bit.
//
//    <filerate>     file control rate to assume [int]
//    <format>       "double", "float", "int64", "int32", "int16" or "byte"
//    <swap>         whether to byte-swap data [1: yes, 0: no]
//
//    If the file has no header, and you don't provide these arguments, the
//    code assumes that <filerate> is the same as the current control rate,
//    <format> is "float", and <swap> is zero.
//                                                               JGG, 3/15/05

static RTNumberPField *_datafile_usage()
{
	die("makeconnection (datafile)",
		"Usage: makeconnection(\"datafile\", filename, lag, "
		"[filerate, format, swap])");
	return NULL;
}

static RTNumberPField *
create_pfield(const Arg args[], const int nargs)
{
	if (nargs < 2)
		return _datafile_usage();

	const char *filename = args[0];
	if (filename == NULL || filename[0] == 0)
		return _datafile_usage();

	double lag;
	if (args[1].isType(DoubleType))
		lag = args[1];
	else
		return _datafile_usage();
	if (lag < 0.0 || lag > 100.0) {
		die("makeconnection (datafile)", "<lag> must be between 0 and 100.");
		return NULL;
	}
	lag *= 0.01;

	double timefactor = 1.0;
	int filerate = -1;
	int formatcode = -1;
	bool swap = false;

	// Handle the optional arguments.
	if (nargs > 2) {
		timefactor = args[2];
		if (timefactor <= 0.0) {
			die("makeconnection (datafile)",
								"<timefactor> must be greater than zero.");
			return NULL;
		}
		if (nargs == 6) {
			filerate = (int) args[3];
			formatcode = DataFile::formatStringToCode(args[4]);
			if (formatcode == -1) {
				warn("makeconnection (datafile)", "Invalid format string. "
								"Valid strings are:");
				warn("makeconnection (datafile)", "\"double\", \"float\", "
								"\"int64\", \"int32\", \"int16\", \"byte\"");
				formatcode = kDataFormatFloat;
			}
			swap = (bool) ((int) args[5]);
		}
	}

	DataFileReaderPField *pfield;

	if (formatcode == -1)	// use defaults in the PField
		pfield = new DataFileReaderPField(filename, lag, resetval, timefactor);
	else
		pfield = new DataFileReaderPField(filename, lag, resetval, timefactor,
							filerate, formatcode, swap);

	return pfield;
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

