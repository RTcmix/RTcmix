/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <ugens.h>		// for warn, die

// Functions for creating signal conditioning wrapper PFields.   -John Gibson, 11/25/04

extern int resetval;		// declared in src/rtcmix/minc_functions.c


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
	Handle makefilter(const Arg args[], const int nargs);
}


// -------------------------------------------------------------- makefilter ---

enum {
	kSmoothFilter,
	kQuantizeFilter,
	kFitRangeFilter
};

static Handle
_makefilter_usage()
{
	die("makefilter",
		"\n   usage: filt = makefilter(pfield, \"fitrange\", min, max [, \"bipolar\"])"
		"\nOR"
		"\n   usage: filt = makefilter(pfield, \"quantize\", quantum)"
		"\nOR"
		"\n   usage: filt = makefilter(pfield, \"smooth\", lag)"
		"\n");
	return NULL;
}

Handle
makefilter(const Arg args[], const int nargs)
{
	if (nargs < 3)
		return _makefilter_usage();

	PField *innerpf = (PField *) args[0];
	if (innerpf == NULL)
		return _makefilter_usage();

	int type;
	if (args[1].isType(StringType)) {
		if (args[1] == "smooth" || args[1] == "lowpass")
			type = kSmoothFilter;
		else if (args[1] == "quantize")
			type = kQuantizeFilter;
		else if (args[1] == "fitrange")
			type = kFitRangeFilter;
		else {
			die("makefilter", "Unsupported filter type \"%s\".",
								(const char *) args[1]);
			return NULL;
		}
	}
	else
		return _makefilter_usage();

	PField *arg1pf = (PField *) args[2];
	if (arg1pf == NULL) {
		if (args[2].isType(DoubleType))
			arg1pf = new ConstPField((double) args[2]);
		else
			return _makefilter_usage();
	}

	PField *arg2pf = NULL;
	if (nargs > 3) {
		arg2pf = (PField *) args[3];
		if (arg2pf == NULL) {
			if (args[3].isType(DoubleType))
				arg2pf = new ConstPField((double) args[3]);
			else
				return _makefilter_usage();
		}
	}

	PField *filt;
	if (type == kSmoothFilter)
		filt = new SmoothPField(innerpf, resetval, arg1pf);
	else if (type == kQuantizeFilter)
		filt = new QuantizePField(innerpf, arg1pf);
	else if (type == kFitRangeFilter) {
		if (arg2pf) {
			if (nargs > 4 && args[4] == "bipolar")
				filt = new RangePField(innerpf, arg1pf, arg2pf, RangePField::BipolarSource);
			else
				filt = new RangePField(innerpf, arg1pf, arg2pf);
		}
		else
			return _makefilter_usage();
	}

	return _createPFieldHandle(filt);
}

