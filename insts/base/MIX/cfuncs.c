/* functions in here for setline() and reset(), and a profile() to set them up */
#include <stdio.h>
#include "../../Minc/defs.h"
#include "../../H/ugens.h"

int lineset = 0;
int resetval = 200;

double setline(float *p, int n_args)
{
	float pp[MAXDISPARGS];
	int i;

	pp[0] = 1;
	pp[1] = 24;
	pp[2] = 1000;

	for(i = 0; i < n_args; i++) pp[i+3] = p[i];

	makegen(pp, n_args+3);
	lineset = 1;
	return(1.0);
}

double reset(float *p, int n_args)
{
	if(p[0]) resetval = p[0];
	fprintf(stderr,"Envelope calls set to %d times per sec\n",resetval);
	return(1.0);
}


int NBYTES = 32768;

int profile() 
{
	UG_INTRO("setline",setline);
	UG_INTRO("reset",reset);
}

