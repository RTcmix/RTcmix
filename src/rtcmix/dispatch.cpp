/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include <prototypes.h>
#include <ugens.h>
#include <maxdispargs.h>
#include "mixerr.h"
#include <stdio.h>

int
RTcmix::dispatch(const char *func_label, const Arg arglist[], const int nargs, Arg *retval)
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

	int status = RTcmix::dispatch(str, rtcmixargs, n_args, &retarg);
	
	// Handle state returned from dispatch()
	
	if (status == 0) {
		if (retarg.isType(DoubleType)) {
		   retval = (double) retarg;
		}
		else if (retarg.isType(StringType)) {
		   retval = (double) (int) retarg.string();
		}
		else if (retarg.isType(HandleType)) {
			// Retrieve instrument pointer from retval.handle.
			if (inst != NULL) {
				Handle rethandle = (Handle) retarg;
				if (rethandle->type == InstrumentPtrType)
					*inst = (Instrument *) rethandle->ptr;
			}
			retval = 0;
		}
		else {
			fprintf(stderr, "dispatch: unhandled Arg return type!\n");
			retval = 0;
		}
	}
	else
		retval = (double) status;

	delete [] rtcmixargs;

	return retval;
}

// This is an extern "C" wrapper for trees.c

double parse_dispatch(const char *str, double *pp, int n_args, void **inst)
{
	return dispatch(str, pp, n_args, inst);
}

// This is an extern "C" wrapper for RT.c.  Make sure decl matches one in RT.xs

extern "C" {
int perl_dispatch(const char *str, const Arg args[], int n_args, Arg *retarg)
{
	return RTcmix::dispatch(str, args, n_args, retarg);
}
};
