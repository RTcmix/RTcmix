#include <pthread.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rt.h"
#include "../rtstuff/rtdefs.h"
#include "../sys/mixerr.h"
#include <string.h>
#include <iostream.h>
#include "defs.h"

extern rt_item *rt_list;
extern int print_is_on;
extern int rtInteractive;
extern int oldSched;

extern pthread_mutex_t heapLock;

extern void heapSched(Instrument *Iptr);

extern "C" {
double checkInsts(char *fname, double *pp, short n_args)
{
	int i;
	rt_item *rt_p;
	rt_item *rt_temp;
	Instrument *Iptr;
	double rv;
	float p[MAXDISPARGS];

	/* printf("ENTERING checkInsts() FUNCTION -----\n"); */

	/* convert pp to floats */
	for(i = 0; i < n_args; i++) p[i] = (float)pp[i];
	/* and zero out the rest */
	for(i = n_args; i < MAXDISPARGS; i++) p[i] = pp[i] = 0.0;

	mixerr = MX_FNAME;
	rt_temp = rt_list;
	rt_p = rt_list;

	while (rt_p) {
	  
		if((rv=strcmp(rt_p->rt_name,fname)) == 0) {
			if(print_is_on) {
				printf ("========<rt-queueing>=======\n");
				printf ("%s:  ",fname);
				for (i = 0; i < n_args; i++)  printf ("%f ",p[i]);
				printf("\n");
			}

			/* set up the Instrument */
			
			Iptr = (*(rt_p->rt_ptr))();

			Iptr->init(p,n_args);

			/* insert / chop Instrument into heap / chunks */

			if (rtInteractive) {
			  pthread_mutex_lock (&heapLock); // lock mutex variable
			  heapSched(Iptr);
			  pthread_mutex_unlock (&heapLock);  // unlock mutex variable
			}
			// Or else set things up for a heap sort ... the old scheduler
			else if (oldSched){
			  heapify(Iptr);
			}
			// Or just schedule with the new scheduler
			else {
			  heapSched(Iptr);
			}
                        rv = 1;
			mixerr = MX_NOERR;
			rt_list = rt_temp;
			/* printf("EXITING checkInsts() FUNCTION -----\n"); */
			return rv;
		}
		rt_p = rt_p->rt_next;
	}
	rt_list = rt_temp;
	/* printf("EXITING checkInsts() FUNCTION (function not found) -----\n"); */
	return 0; /* This was NULL on SGI's ... not good */
}
}

