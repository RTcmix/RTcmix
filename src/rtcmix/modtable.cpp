/* RTcmix  - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <rtcmix_types.h>
#include <tableutils.h>
#include <PField.h>
#include <utils.h>
#include <ugens.h>		// for warn, die

// Functions for modifying table PFields.   -JGG, 4/8/05

#define NORMALIZE_USAGE "table = modtable(table, \"normalize\" [, peak])"
#define REVERSE_USAGE "table = modtable(table, \"reverse\")"
#define SHIFT_USAGE "table = modtable(table, \"shift\", shift)"


// =============================================================================
// local utilities

static PField *_modtable_usage(const char *msg)
{
	die(NULL, "Usage: %s", msg);
	return NULL;
}


// =============================================================================
// Set up the various table modifications.

// Rescale the values in table so that the maximum absolute value is <peak>.
// If no <peak>, peak is 1.  Result depends on the sign of values in the table.
//
//    sign of values          resulting range of values
//    --------------------------------------------------------------
//    all positive            between 0 and peak
//    all negative            between 0 and -peak
//    positive and negative   between -peak and peak

static PField *
_normalize_table(PField *intable, const Arg args[], const int nargs)
{
	double peak = 1.0;
	if (nargs > 0)
		peak = (double) args[0];
	if (peak == 0.0)
		peak = 0.00001;
	const int len = intable->values();
	const double *array = (double *) *intable;
	double max = 0.0;
	for (int i = 0; i < len; i++) {
		double absval = fabs(array[i]);
		if (absval > max)
			max = absval;
	}
	max /= peak;
	if (max == 0.0)
		return intable;
	PField *factorpf = new ConstPField(1.0 / max);
	return new MultPField(intable, factorpf);
}

static PField *
_reverse_table(PField *intable, const Arg args[], const int nargs)
{
	return new ReversePField(intable);
}

// Shift values of the table by <shift> array locations.  Positive values of
// <shift> shift to the right; negative values to the left.  If a value is
// shifted off the end of the array in either direction, it reenters the other
// end of the array.  Two examples:
//
//    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]      original table, size = 10
//    [7, 8, 9, 0, 1, 2, 3, 4, 5, 6]      shift = 3
//    [3, 4, 5, 6, 7, 8, 9, 0, 1, 2]      shift = -3
//
// Note that the shifting is actually done on-the-fly by ShiftPField.

static PField *
_shift_table(PField *intable, const Arg args[], const int nargs)
{
	if (nargs < 1)
		return _modtable_usage(SHIFT_USAGE);
	const int shift = (int) args[0];
	return new ShiftPField(intable, shift);
}


// =============================================================================
// The remaining functions are public, callable from scripts.

extern "C" {
	Handle modtable(const Arg args[], const int nargs);
}


// ---------------------------------------------------------------- modtable ---
static Handle _modtable_usage()
{
	die("modtable",
		"\n   usage: " NORMALIZE_USAGE
		"\nOR"
		"\n   usage: " REVERSE_USAGE
		"\nOR"
		"\n   usage: " SHIFT_USAGE
		"\n");
	return NULL;
}

Handle modtable(const Arg args[], const int nargs)
{
	if (nargs < 2)
		return _modtable_usage();

	PField *intable = (PField *) args[0];
	if (intable == NULL || !is_table(intable))
		return _modtable_usage();

	PField *outtable = NULL;
	if (args[1].isType(StringType)) {
		if (args[1] == "normalize")
			outtable = _normalize_table(intable, &args[2], nargs - 2);
		else if (args[1] == "reverse")
			outtable = _reverse_table(intable, &args[2], nargs - 2);
		else if (args[1] == "shift")
			outtable = _shift_table(intable, &args[2], nargs - 2);
		else {
			die("modtable", "Unsupported modification \"%s\".",
								(const char *) args[1]);
			return NULL;
		}
	}
	else
		return _modtable_usage();
	return (outtable == NULL) ? NULL : createPFieldHandle(outtable);
}

