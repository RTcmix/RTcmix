#include "../../H/ugens.h"
float basis;

double mp(p,n_args)
float *p;
/* minc function to return pitch +basis mod12 */
{
	float pch;
	int num,oct;
	oct = (int)p[0]/12;
	num = (int)p[0] % 12;
	pch = pchoct((float)num/12. + basis + oct);
printf("%d %f %f %f\n",num,pch,p[0],basis);
	return(pch);
}
mpset(p,n_args)
float *p;
{
	basis = p[0];
}
