#include <stdio.h>
#include "../../Minc/defs.h"
#include "../../H/ugens.h"

int resetval = 1000;

double reset(float *p, int n_args)
{
	if(p[0]) resetval = p[0];
	fprintf(stderr,"Envelope calls set to %d times per sec\n",resetval);
	return(1.0);
}


int NBYTES = 32768;

profile()
{
	UG_INTRO("reset",reset);
}
