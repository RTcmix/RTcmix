#include "../H/ugens.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>

double
m_sampfunc(p,n_args)

float *p;
short n_args;

{
	int size, fnumber,skipin;
	float *thefunct;

	/* p0 == number of the function, p1 == amount to skip */


	fnumber = p[0];
	size = fsize(fnumber);
	skipin = (p[1] < size) ? p[1] : size - 1; /* to avoid segvs */
	thefunct = (float *) floc(fnumber);
	return(thefunct[skipin]);
}

double
m_sampfunci(p,n_args)	/* interpolated version of sampfunc -- DAS 5/90 */

float *p;
short n_args;

{
	int fnumber,size,skipin, skipin2;
	float *thefunct, frac;

	/* p0 == number of the function, p1 == amount to skip */

	fnumber = p[0];
	skipin = p[1];
	frac = p[1] - skipin;
	size = fsize(fnumber);
	if(skipin >= size-1) {         /* was (skipin >= size-2)  --JGG 7/99 */
		skipin = size - 1;
		skipin2 = skipin;
	}
	else
		skipin2 = skipin + 1;
	thefunct = (float *)floc(fnumber);
	return(thefunct[skipin] + frac * (thefunct[skipin2]-thefunct[skipin]));
}

