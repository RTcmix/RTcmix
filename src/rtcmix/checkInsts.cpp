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

   if (Option::print()) {
      printf("========<rt-queueing>=======\n");
      printf("%s:  ", instname);
      for (i = 0; i < nargs; i++) {
         arglist[i].printInline(stdout);
      }
      putchar('\n');
      fflush(stdout);
   }
}


double
RTcmix::checkInsts(const char *instname, const Arg arglist[], 
				   const int nargs, Arg *retval)
{
// FIXME: set this up as in addcheckfuncs, so that the guts of the
// instrument name list are not exposed here.  -JGG
   rt_item *rt_p;       // rt_item defined in rt.h
   rt_item *rt_temp;
   Instrument *Iptr;

#ifdef DEBUG
   printf("ENTERING checkInsts() FUNCTION -----\n");
#endif
	
	if (!rtsetparams_was_called()) {
		die(instname, "You did not call rtsetparams!");
		return -1;
	}

   mixerr = MX_FNAME;
   rt_temp = rt_list;
   rt_p = rt_list;
   
   *retval = 0.0;	// Default to float 0

   while (rt_p) {
      if (strcmp(rt_p->rt_name, instname) == 0) {

         ::printargs(instname, arglist, nargs);

         /* Create the Instrument */
         
         Iptr = (*(rt_p->rt_ptr))();
		 
		 if (!Iptr) {
		 	mixerr = MX_FAIL;
			return -1;
		 }

         Iptr->ref();   // We do this to assure one reference
   
		// Load PFieldSet with ConstPField instances for each 
		// valid p field.
         PFieldSet *pfieldset = new PFieldSet(nargs);
		 if (!pfieldset) {
			Iptr->unref();
		 	mixerr = MX_FAIL;
			return -1;
		 }
         for (int arg = 0; arg < nargs; ++arg) {
			const Arg &theArg = arglist[arg];
			if (theArg.isType(DoubleType))
				pfieldset->load(new ConstPField((double) theArg), arg);
			else if (theArg.isType(StringType))
				pfieldset->load(new StringPField(theArg.string()), arg);
			else if (theArg.isType(HandleType)) {
				Handle handle = (Handle) theArg;
				if (handle != NULL) {
				   if (handle->type == PFieldType) {
				      assert(handle->ptr != NULL);
				      pfieldset->load((PField *) handle->ptr, arg);
				   }
				   else {
				 	  fprintf(stderr, "%s: arg %d: Unsupported handle type!\n",
						   	  instname, arg);
			 	  	  mixerr = MX_FAIL;
				   }
			    }
			    else {
				   fprintf(stderr, "%s: arg %d: NULL handle!\n",
						   instname, arg);
			 	   mixerr = MX_FAIL;
			    }
			}
			else if (theArg.isType(ArrayType)) {
			   fprintf(stderr, "%s: arg %d: Array (list) types cannot be passed to instruments.\n\tUse maketable(\"literal\", ...) instead.\n",
					   instname, arg);
			   mixerr = MX_FAIL;
			}
			else {
			   fprintf(stderr, "%s: arg %d: Illegal argument type!\n",
					   instname, arg);
			   mixerr = MX_FAIL;
			}
		}
		
		if (mixerr == MX_FAIL) {
			delete pfieldset;
			Iptr->unref();
			return -1.0;
		}
		
        double rv = (double) Iptr->setup(pfieldset);

        if (rv != (double) DONT_SCHEDULE) { // only schedule if no setup() error
			// For non-interactive case, configure() is delayed until just
			// before instrument run time.
			if (rtInteractive) {
			   if (Iptr->configure(RTBUFSAMPS) != 0) {
				   rv = DONT_SCHEDULE;	// Configuration error!
			   }
			}
		}

		// Clean up if there was an error.
		if (rv == (double) DONT_SCHEDULE) {
			Iptr->unref();
			mixerr = MX_FAIL;
            return rv;
		}

        /* schedule instrument */
        Iptr->schedule(rtHeap);

        mixerr = MX_NOERR;
        rt_list = rt_temp;

		// Create Handle for Iptr on return
		*retval = createInstHandle(Iptr);
#ifdef DEBUG
         printf("EXITING checkInsts() FUNCTION -----\n");
#endif
         return rv;
      }
      rt_p = rt_p->rt_next;
   }
   rt_list = rt_temp;

   return 0.0;
}

