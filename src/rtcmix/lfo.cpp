/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <ugens.h>		// for warn, die

// Functions for creating LFO PFields.    -John Gibson, 11/20/04

extern int resetval;		// declared in src/rtcmix/minc_functions.c

extern "C" {
Handle maketable(const Arg args[], const int nargs); // defined in table.cpp
}


// --------------------------------------------------------- local utilities ---
static Handle
_createPFieldHandle(PField *pfield)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	handle->type = PFieldType;
	handle->ptr = (void *) pfield;
	return handle;
}

// create an appropriate TablePField from string description
static TablePField *
_makewavetable(const char *wavetype)
{
	const int tabsize = 2000;
	const int numpartials = 21;
	const int nargs = numpartials + 2;

	Arg *mtargs = new Arg[nargs];
	mtargs[0] = "wave";
	mtargs[1] = tabsize;

	if (strncmp(wavetype, "sine", 3) == 0) {
		mtargs[2] = 1.0;
		for (int i = 3; i < nargs; i++)
			mtargs[i] = 0.0;
	}
	else if (strncmp(wavetype, "saw", 3) == 0) {
		int partial = 1;
		for (int i = 2; partial <= numpartials; i++, partial++)
			mtargs[i] = 1.0 / (double) partial;
	}
	else if (strncmp(wavetype, "squ", 3) == 0) {
		int partial = 1;
		for (int i = 2; partial <= numpartials; i++, partial++) {
			if (partial % 2)
				mtargs[i] = 1.0 / (double) partial;
			else
				mtargs[i] = 0.0;
		}
	}
	else if (strncmp(wavetype, "tri", 3) == 0) {
		int partial = 1;
		for (int i = 2; partial <= numpartials; i++, partial++) {
			if (partial % 2)
				mtargs[i] = 1.0 / (double) (partial * partial);
			else
				mtargs[i] = 0.0;
		}
	}
	else {
		die("makeLFO", "Waveform string can be \"sine\", \"saw\", "
						"\"square\" or \"triangle\"; or pass a table handle.");
		delete [] mtargs;
		return NULL;
	}

	Handle handle = maketable(mtargs, nargs);
	delete [] mtargs;

	return (TablePField *) handle->ptr;
}


// ----------------------------------------------------------------- makeLFO ---

extern "C" {
	Handle makeLFO(const Arg args[], const int nargs);
}

static void
_makeLFO_usage()
{
	die("makeLFO",
		"\n   usage: lfo = makeLFO(wave, [interpolation,] freq, amp)");
}

typedef enum {
	kTruncate,
	kInterp1stOrder
} InterpType;

Handle
makeLFO(const Arg args[], const int nargs)
{
	if (nargs < 3) {
		_makeLFO_usage();
		return NULL;
	}

	TablePField *wavetablePF = (TablePField *) ((PField *) args[0]);
	if (wavetablePF == NULL || wavetablePF->values() < 2) {	// not a TablePField
		if (args[0].isType(StringType)) {
			wavetablePF = _makewavetable(args[0]);
			if (wavetablePF == NULL)
				return NULL;
		}
		else {
			_makeLFO_usage();
			return NULL;
		}
	}
	double *wavetable = (double *) *wavetablePF;
	int len = wavetablePF->values();

	InterpType interp = kInterp1stOrder;

	int index = 1;
	if (args[index].isType(StringType)) {
		if (args[index] == "nointerp")
			interp = kTruncate;
		else if (args[index] == "interp")
			interp = kInterp1stOrder;
		else {
			die("makeLFO", "Invalid string option \"%s\".",
													(const char *) args[index]);
			return NULL;
		}
		index++;
	}

	PField *freqpf = (PField *) args[index];
	if (freqpf == NULL) {
		if (args[index].isType(DoubleType))
			freqpf = new ConstPField((double) args[index]);
		else {
			_makeLFO_usage();
			return NULL;
		}
	}
	index++;

	PField *amppf = (PField *) args[index];
	if (amppf == NULL) {
		if (args[index].isType(DoubleType))
			amppf = new ConstPField((double) args[index]);
		else {
			_makeLFO_usage();
			return NULL;
		}
	}
	index++;

	PField *lfo;
	if (interp == kInterp1stOrder)
		lfo = new LFOPField(resetval, wavetable, len, freqpf, amppf);
	else // interp == kTruncate
		lfo = new LFOPField(resetval, wavetable, len, freqpf, amppf, false);

	return _createPFieldHandle(lfo);
}


