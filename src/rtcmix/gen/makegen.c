#include "../H/ugens.h"
#include <stdio.h>

int   ngens = 0;       /* total number of gens so far */

/* ok, TOTGENS is the absolute total number of gens we can have in
   a given run, MAXGENS is the number of simultaneous gens we can
   have.  TOTGENS should probably be replaced by an appropriate malloc */

#define TOTGENS 10000
#define MAXGENS 300

float *farrays[TOTGENS];
int sizeof_farray[TOTGENS];
int f_goto[MAXGENS]; /* this is used to guarantee unique farrays in rtcmix */

double
makegen(p,n_args)
float *p;
{
	struct gen gen;
	char   *valloc();

	/* p0=storage loc, p1=gen no, p2=size, p3--> = args */

	int genno = p[0];
	if (genno < 0) genno = -genno;

	if (genno >=MAXGENS) {
		fprintf(stderr, "makegen: no more simultaneous gens available!\n");
		closesf();
		}

	/* makegen now creates a new function for *every* call to it - this
	   is so that we can guarantee the correct version of a given function
	   at run-time during RT operation.  the f_goto[] array keeps track
	   of where to map a particular function table number during the
	   queueing of RT Instruments */
	f_goto[genno] = ngens;

	gen.size = p[2];
	if((farrays[f_goto[genno]] = (float *) valloc((unsigned) gen.size * FLOAT))  == NULL) {
		fprintf(stderr, "makegen: Can't get any function space.\n");
		closesf();
		}
	ngens++;

	sizeof_farray[f_goto[genno]] = p[2];
	
	gen.nargs  = n_args-3;
	gen.pvals = p+3;
	gen.array = farrays[f_goto[genno]];
	gen.slot = (int)p[0];

	switch ((int)p[1]) {
		case 25: gen25(&gen); break;
		case 24: gen24(&gen); break;
		case 20: gen20(&gen); break;
		case 18: gen18(&gen); break;
		case 17: gen17(&gen); break;
		case 10: gen10(&gen); break;
		case  7: gen7(&gen); break;
		case  5: gen5(&gen); break;
		case  9: gen9(&gen); break;
		case  6: gen6(&gen); break;
		case  3: gen3(&gen); break;
		case  2: gen2(&gen); break;
		default: fprintf(stderr,"makegen: There is no gen%d\n",(int)p[1]);
	}
    return 0;
}
