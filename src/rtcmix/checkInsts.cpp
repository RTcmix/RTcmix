/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <prototypes.h>
#include <maxdispargs.h>
#include <pthread.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/PField.h"
#include "../rtstuff/PFieldSet.h"
#include "../rtstuff/rt.h"
#include "../rtstuff/rtdefs.h"
#include "../sys/mixerr.h"
#include <stdio.h>
#include <string.h>

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
         arg = arglist[i];
         switch (arg.type) {
            case FloatType:
               printf("%g ", arg.val.number);
               break;
            case StringType:
               printf("\"%s\" ", arg.val.string);
               break;
            case HandleType:
               printf("Handle:%p ", arg.val.handle);
               break;
            case ArrayType:
               printf("[%g...%g] ", arg.val.array->data[0],
                              arg.val.array->data[arg.val.array->len - 1]);
               break;
            default:
               break;
         }
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
		  const Arg *pArg = &arglist[arg];
		  switch (pArg->type) {
    		 case FloatType:
        		pfieldset->load(new ConstPField((double) pArg->val.number), arg);
        		break;
    		 case StringType:
        		break;
    		 case HandleType:
        		pfieldset->load((PField *) pArg->val.handle, arg);
        		break;
			 case ArrayType:
				pfieldset->load(new TablePField(pArg->val.array->data,
												pArg->val.array->len),
								arg);
        		break;
    		 default:
        		break;
		  }
		}
         double rv = (double) Iptr->setup(pfieldset);

         if (rv != (double) DONT_SCHEDULE) { // only schedule if no init() error
            // For non-interactive case, configure() is delayed until just
            // before instrument run time.
            if (rtInteractive)
               Iptr->configure();

            /* schedule instrument */
            Iptr->schedule(&rtHeap);

            mixerr = MX_NOERR;
            rt_list = rt_temp;
         }
         else
            return rv;

#ifdef DEBUG
         printf("EXITING checkInsts() FUNCTION -----\n");
#endif

//FIXME: ??need to create Handle for Iptr on return
//       and return double rv?  can't be done in parse_dispatch now!

         return rv;
      }
      rt_p = rt_p->rt_next;
   }
   rt_list = rt_temp;

   return 0.0;
}

#else /* !PFIELD_CLASS */

double checkInsts(char *fname, double *pp, int n_args, void **inst)
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
            if (rtInteractive)
               Iptr->configure();

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
