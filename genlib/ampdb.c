#include <math.h>

double ampdb(float db)
{
	return(pow(10.,(double)(db/20.)));
}

double dbamp(float amp)    /* added by JG, 3/25/00 */
{
   double fabs_amp = fabs((double) amp);
   return (20.0 * log10(fabs_amp));
}

