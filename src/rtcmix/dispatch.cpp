/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <prototypes.h>
#include <ugens.h>
#include <maxdispargs.h>
#include "../sys/mixerr.h"
#include <stdio.h>

#ifdef PFIELD_CLASS

int
dispatch(const char *func_label, const Arg arglist[], const int nargs, Arg *retval)
{
   /* Search non-rt and rt function lists for a match with <func_label>.
      If there is a match of either, checkfunc or checkInsts will call
      the appropriate function and return zero.  If both of these return
      non-zero status, print an error message.
   */
   int status = checkfunc(func_label, arglist, nargs, retval);

   if (status != 0) {         /* search rt functions */
      mixerr = MX_NOERR;      /* clear old errors */
      checkInsts(func_label, arglist, nargs, retval);
      if (mixerr == MX_FNAME)
         advise(NULL, "Note: \"%s\" is an undefined function or instrument.",
                                                                  func_label);
	  else
	  	status = 0;
   }

   return status;
}

#include <stdlib.h>

class Instrument;

double
dispatch(const char *str, double *pp, int n_args, void **inst)
{
	Arg retarg;
	double retval;

	Arg *rtcmixargs = new Arg[n_args];
	if (rtcmixargs == NULL) {
	  fprintf(stderr, "dispatch: out of memory\n");
	  return -1.0;
	}
	for (int i = 0; i < n_args; i++) {
	  rtcmixargs[i] = pp[i];
	}

	int status = dispatch(str, rtcmixargs, n_args, &retarg);
	
	// Handle state returned from dispatch()
	
	if (status == 0) {
		if (retarg.getType() == DoubleType) {
		   retval = (double) retarg;
		}
		else if (retarg.getType() == StringType) {
		   retval = (double) (int) (const char *) retarg;
		}
		else if (retarg.getType() == HandleType) {
			// Retrieve instrument pointer from retval.handle.
			if (inst != NULL) {
				Handle rethandle = (Handle) retarg;
				if (rethandle->type == InstrumentPtrType)
					*inst = (Instrument *) rethandle->ptr;
			}
		}
		else {
			fprintf(stderr, "dispatch: unhandled Arg return type!\n");
		}
	}
	else
		retval = (double) status;

	delete [] rtcmixargs;

	return retval;
}

#else /* !PFIELD_CLASS */

/* This is the function -- called by the parser, the socket interface,
   and the imbRTcmix.o imbeddable object -- that lets RTcmix perform
   some action described by a command <str> and its associated pfields
   <pp>.  There are <n_args> elements in the <pp> array.  If <str> is
   not on the symbol list for non-RT functions (including legacy
   disk-based instruments), then we ask a C++ function, checkInsts, to
   look at the command.  If checkInsts thinks it's a real-time instrument,
   it inits and queues the instrument and passes back a pointer to the
   Instrument object, cast as a void pointer.  (We do this to avoid
   the mess of having parse_dispatch know about a C++ class.)  The
   caller to parse_dispatch can retrieve the Instrument pointer by
   passing in a void pointer, <inst>, by reference, and casting it to
   an Instrument pointer on return from parse_dispatch.  It the caller
   doesn't care about Instrument pointers, it can just supply NULL as
   the fourth argument to parse_dispatch.

   Example:

      void        *phantom;
      Instrument  *instPtr;
      double      pp[512];

      pp[0] = 0.0; pp[1] = 1.0; pp[2] = 10000;

      doubleval = dispatch("foo", pp, 3, &phantom);   // for RT inst
      instPtr = (Instrument *) phantom;

      // for Minc func, legacy inst, or if we just don't need ptr to RT inst
      doubleval = dispatch("bar", pp, 0, NULL);

   dispatch returns whatever double is returned by the RTcmix
   instrument or function; or, if <str> is not recognized as a valid
   RTcmix instrument or function name, it returns 0 and gives a warning.

   This comment is now longer than the function, so it's time to stop!

   -JGG, 12/12/02
*/
double dispatch(const char *str, double *pp, int n_args, void **inst)
{
	double rv;

	if (inst != NULL)
		*inst = NULL;

	mixerr = MX_NOERR;      /* Clear up old errors */

	/* Find and call the routine that goes with named function (non-rt!) */
	rv = checkfuncs(str, pp, n_args);
	if (mixerr == MX_NOERR)
		return rv;

	/* Else check to see if it's a real-time Instrument symbol.
	   Note that *inst must be cast to Instrument * by caller. */
	rv = checkInsts(str, pp, n_args, inst);

	if (mixerr == MX_FNAME) {
		advise(NULL, "(Note: %s() is an undefined function or Instrument.)", str);
		return 0.0;
	}

	return rv;
}

#endif /* !PFIELD_CLASS */
