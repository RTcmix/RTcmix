/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for 
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <prototypes.h>
#include <maxdispargs.h>
#include <pthread.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rt.h"
#include "../rtstuff/rtdefs.h"
#include "../sys/mixerr.h"
#include <stdio.h>
#include <string.h>

extern heap rtHeap;		// intraverse.C

//#define DEBUG

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

			Iptr->ref();	// We do this to assure one reference
	
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

