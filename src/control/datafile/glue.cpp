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

extern int resetval;		// declared in src/rtcmix/minc_functions.c


// -----------------------------------------------------------------------------
//
//    stream = makeconnection("datafile", filename, lag,
//                                   [filerate, format, swap])
//
//    <filename>     full or relative path to data file [string]
//    <lag>          amount of smoothing for value stream [percent: 0-100]
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

static int format_string_to_code(const char *str)
{
	if (strcmp(str, "double") == 0)
		return kDataFormatDouble;
	else if (strcmp(str, "float") == 0)
		return kDataFormatFloat;
	else if (strcmp(str, "int64") == 0)
		return kDataFormatInt64;
	else if (strcmp(str, "int32") == 0)
		return kDataFormatInt32;
	else if (strcmp(str, "int16") == 0)
		return kDataFormatInt16;
	else if (strcmp(str, "byte") == 0)
		return kDataFormatByte;
	return -1;
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

	int filerate = -1;
	int formatcode = -1;
	bool swap = false;

	// Handle the optional arguments.
	if (nargs == 5) {
		filerate = (int) args[1];
		const char *format = args[2];
		formatcode = format_string_to_code(format);
		swap = (bool) ((int) args[3]);
	}

	DataFileReaderPField *pfield;

	if (formatcode == -1)	// use defaults in the PField
		pfield = new DataFileReaderPField(filename, lag, resetval);
	else
		pfield = new DataFileReaderPField(filename, lag, resetval,
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

