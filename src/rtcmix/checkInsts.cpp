/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <prototypes.h>
#include <maxdispargs.h>
#include <pthread.h>
#include <Instrument.h>
#include <PField.h>
#include <PFieldSet.h>
#include "../rtstuff/rt.h"
#include "../rtstuff/rtdefs.h"
#include "../sys/mixerr.h"
#include <rtcmix_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern heap rtHeap;     // intraverse.C

//#define DEBUG

#ifdef PFIELD_CLASS

static void
_printargs(const char *instname, const Arg arglist[], const int nargs)
{
   int i;
   Arg arg;

   if (print_is_on) {
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
checkInsts(const char *instname, const Arg arglist[], const int nargs, Arg *retval)
{
// FIXME: set this up as in addcheckfuncs, so that the guts of the
// instrument name list are not exposed here.  -JGG
   rt_item *rt_p;       // rt_item defined in rt.h
   rt_item *rt_temp;
   Instrument *Iptr;

#ifdef DEBUG
   printf("ENTERING checkInsts() FUNCTION -----\n");
#endif

   mixerr = MX_FNAME;
   rt_temp = rt_list;
   rt_p = rt_list;

   while (rt_p) {
     
      if (strcmp(rt_p->rt_name, instname) == 0) {

         _printargs(instname, arglist, nargs);

         /* set up the Instrument */
         
         Iptr = (*(rt_p->rt_ptr))();

         Iptr->ref();   // We do this to assure one reference
   
		// Load PFieldSet with ConstPField instances for each 
		// valid p field.
		PFieldSet *pfieldset = new PFieldSet(nargs);
		for (int arg = 0; arg < nargs; ++arg) {
		  const Arg &theArg = arglist[arg];
		  if (theArg.isType(DoubleType))
			pfieldset->load(new ConstPField((double) theArg), arg);
		  else if (theArg.isType(StringType))
			pfieldset->load(new StringPField(theArg.string()), arg);
		  else if (theArg.isType(HandleType)) {
			Handle handle = (Handle) theArg;
			if (handle->type == PFieldType) {
				assert(handle->ptr != NULL);
				pfieldset->load((PField *) handle->ptr, arg);
			}
		  }
		  else {
			// For now, default to using a zero PField.
			pfieldset->load(new ConstPField(0.0), arg);
			break;
		  }
		}
        double rv = (double) Iptr->setup(pfieldset);

        if (rv == (double) DONT_SCHEDULE) { // only schedule if no init() error
            return rv;
		}
		
        // For non-interactive case, configure() is delayed until just
        // before instrument run time.
		if (rtInteractive) {
		   if (Iptr->configure(RTBUFSAMPS) != 0)
			   return -1;	// Configuration error!
		}

        /* schedule instrument */
        Iptr->schedule(&rtHeap);

        mixerr = MX_NOERR;
        rt_list = rt_temp;

		// Create Handle for Iptr on return
		Handle rethandle = (Handle) malloc(sizeof(struct _handle));
		if (rethandle) {
			rethandle->type = InstrumentPtrType;
			rethandle->ptr = (void *) Iptr;
			*retval = rethandle;
		}
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

#else /* !PFIELD_CLASS */

double checkInsts(const char *fname, double *pp, int n_args, void **inst)
{
   int i;
   rt_item *rt_p;
   rt_item *rt_temp;
   Instrument *Iptr;
   double rv;
   float p[MAXDISPARGS];

#ifdef DEBUG
   printf("ENTERING checkInsts() FUNCTION -----\n");
#endif

   /* convert pp to floats */
   for (i = 0; i < n_args; i++) p[i] = (float)pp[i];
   /* and zero out the rest */
   for (i = n_args; i < MAXDISPARGS; i++) p[i] = pp[i] = 0.0;

   mixerr = MX_FNAME;
   rt_temp = rt_list;
   rt_p = rt_list;

   while (rt_p) {
     
      if (strcmp(rt_p->rt_name, fname) == 0) {
         if (print_is_on) {
            printf("========<rt-queueing>=======\n");
            printf("%s:  ",fname);
            for (i = 0; i < n_args; i++)
               printf("%f ",p[i]);
            printf("\n");
         }

         /* set up the Instrument */
         
         Iptr = (*(rt_p->rt_ptr))();

         Iptr->ref();   // We do this to assure one reference
   
         rv = (double) Iptr->init(p, n_args, pp);

         if (rv != DONT_SCHEDULE) { // only schedule if no init() error
            // For non-interactive case, configure() is delayed until just
            // before instrument run time.
            if (rtInteractive) {
               if (Iptr->configure(RTBUFSAMPS) != 0)
				   return -1;	// Configuration error!
			}

            /* schedule instrument */
            Iptr->schedule(&rtHeap);

            mixerr = MX_NOERR;
            rt_list = rt_temp;
         } else {
            return rv;
         }

#ifdef DEBUG
         printf("EXITING checkInsts() FUNCTION -----\n");
#endif

         if (inst != NULL)
            *inst = (void *) Iptr;
         return rv;
      }
      rt_p = rt_p->rt_next;
   }
   rt_list = rt_temp;

#ifdef DEBUG
   printf("EXITING checkInsts() FUNCTION (function not found) -----\n");
#endif

   return 0.0;
}

#endif /* !PFIELD_CLASS */
