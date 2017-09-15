/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include "prototypes.h"
#include <maxdispargs.h>
#include <pthread.h>
#include <Instrument.h>
#include <PField.h>
#include <PFieldSet.h>
#include "utils.h"
#include "rt.h"
#include "rtdefs.h"
#include "mixerr.h"
#include "rtcmix_types.h"
#include "ugens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <Option.h>

//#define DEBUG

static void
printargs(const char *instname, const Arg arglist[], const int nargs)
{
   int i;
   Arg arg;

   if (Option::print() >= MMP_PRINTALL) {
      RTPrintf("========<rt-queueing>=======\n");
      RTPrintfCat("%s:  ", instname);
      for (i = 0; i < nargs; i++) {
         arglist[i].printInline(stdout);
      }
      RTPrintf("\n");
   }
}

static InstCreatorFunction
findInstCreator(rt_item *inList, const char *inInstName)
{
	rt_item *item = inList;       // rt_item defined in rt.h
	while (item) {
		if (strcmp(item->rt_name, inInstName) == 0) {
			return item->rt_ptr;
		}
		item = item->rt_next;
	}
	return NULL;
}

// Load the argument list into a PFieldSet, hand to instrument, and call setup().  Does not destroy
// the instrument on failure.

static int loadPFieldsAndSetup(const char *inName, Instrument *inInst, const Arg arglist[], const int nargs)
{
	// Load PFieldSet with ConstPField instances for each
	// valid p field.
	mixerr = MX_NOERR;
	PFieldSet *pfieldset = new PFieldSet(nargs);
	if (!pfieldset) {
		mixerr = MX_EMEM;
		return DONT_SCHEDULE;
	}
	for (int arg = 0; arg < nargs; ++arg) {
		const Arg &theArg = arglist[arg];
		switch (theArg.type()) {
			case DoubleType:
				pfieldset->load(new ConstPField((double) theArg), arg);
				break;
			case StringType:
				pfieldset->load(new StringPField(theArg.string()), arg);
				break;
			case HandleType:
			{
				Handle handle = (Handle) theArg;
				if (handle != NULL) {
					if (handle->type == PFieldType) {
						assert(handle->ptr != NULL);
						pfieldset->load((PField *) handle->ptr, arg);
					}
					else if (handle->type == InstrumentPtrType) {
						assert(handle->ptr != NULL);
						pfieldset->load(new InstPField((Instrument *)handle->ptr), arg);
					}
					else {
						rtcmix_warn(inName, "arg %d: Unsupported handle type!", arg);
						mixerr = MX_FAIL;
					}
				}
				else {
					rtcmix_warn(inName, "arg %d: NULL handle!", arg);
					mixerr = MX_FAIL;
				}
			}
				break;
			case ArrayType:
			{
				Array *array = (Array *)theArg;
				assert(array->data != NULL);
				double *dataCopy = new double[array->len];
				if (dataCopy == NULL) {
					rtcmix_warn(inName, "arg %d: ran out of memory copying array!", arg);
					return MX_EMEM;
				}
				for (unsigned n = 0; n < array->len; ++n)
					dataCopy[n] = array->data[n];
				pfieldset->load(new TablePField(dataCopy, array->len, TablePField::Interpolate2ndOrder), arg);
			}
				break;
			default:
				rtcmix_warn(inName, "arg %d: Illegal argument type!", arg);
				mixerr = MX_FAIL;
				break;
		}
	}
	if (mixerr != MX_NOERR) {
		delete pfieldset;
		return DONT_SCHEDULE;
	}
    return inInst->setup(pfieldset) >= 0 ? 0 : DONT_SCHEDULE;
}

int
RTcmix::checkInsts(const char *instname, const Arg arglist[],
				   const int nargs, Arg *retval)
{
	Instrument *Iptr = NULL;;

#ifdef DEBUG
   RTPrintf("ENTERING checkInsts() FUNCTION -----\n");
#endif

	mixerr = MX_FNAME;

	*retval = 0.0;	// Default to float 0

	InstCreatorFunction instCreator = findInstCreator(rt_list, instname);

	if (instCreator) {
		
		printargs(instname, arglist, nargs);

		if (!rtsetparams_was_called()) {
            mixerr = MX_FAIL;
#ifdef EMBEDDED
			return die(instname, "You need to start the audio device before doing this.");
#else
			return die(instname, "You did not call rtsetparams!");
#endif
		}
		
		/* Create the Instrument */

		Iptr = (*instCreator)();

		if (!Iptr) {
			mixerr = MX_FAIL;
			return DONT_SCHEDULE;
		}

		Iptr->ref();   // We do this to assure one reference

		int rv = loadPFieldsAndSetup(instname, Iptr, arglist, nargs);
		
        if (rv == 0) { // only schedule if no setup() error
			// For non-interactive case, configure() is delayed until just
			// before instrument run time.
			if (rtInteractive) {
			   if ((rv = Iptr->configure(RTBUFSAMPS)) != 0) {
				   mixerr = MX_FAIL;	// Configuration error!
			   }
			}
		}
		// Clean up if there was an error.
		else {
			Iptr->unref();
			if (mixerr == MX_NOERR)		// don't overwrite existing error
				mixerr = MX_FAIL;
			*retval = (Handle) NULL;
			return rv;
		}

		/* schedule instrument */
		Iptr->schedule(rtHeap);

		mixerr = MX_NOERR;

		// Create Handle for Iptr on return
		*retval = createInstHandle(Iptr);
#ifdef DEBUG
		 RTPrintf("EXITING checkInsts() FUNCTION -----\n");
#endif
		 return rv;
	}

   return 0;
}

static Handle mkusage()
{
	die("makeinstrument", "Usage: makeinstrument(\"INSTRUMENTNAME\", p[0], p[1], ...)");
	return 0;
}

extern "C" {
	Handle makeinstrument(const Arg args[], const int nargs);
}

// makeinstrument():  Create an instrument and return it as a handle.  Do not configure or schedule the instrument.
// This is specifically for use with the CHAIN instrument.

Handle
makeinstrument(const Arg arglist[], const int nargs)
{
	if (nargs < 4) {
		return mkusage();
	}
	
	Instrument *Iptr = NULL;;
	const char *instName = (const char *) arglist[0];
	
	if (!instName)
		return mkusage();
	
	InstCreatorFunction instCreator = findInstCreator(RTcmix::rt_list, instName);
	
	if (instCreator) {
		/* Create the Instrument */
		
		Iptr = (*instCreator)();
		
		if (!Iptr) {
			mixerr = MX_FAIL;
			return 0;
		}
		
		Iptr->ref();   // We do this to assure one reference
		
		// Call same function as for checkInsts(), but strip off initial instrument name arg.
		
		double rv = loadPFieldsAndSetup(instName, Iptr, &arglist[1], nargs-1);
		
		Iptr->configureEndSamp(NULL);
		
        if (rv == (double) DONT_SCHEDULE) {
			Iptr->unref();
			mixerr = MX_FAIL;
			return 0;
		}
		mixerr = MX_NOERR;
		
		// Create Handle for Iptr on return
		return createInstHandle(Iptr);
	}
	else {
		rtcmix_advise("makeinstrument",
					  "\"%s\" is an undefined function or instrument.",
					  instName);
	}
	return 0;
}
