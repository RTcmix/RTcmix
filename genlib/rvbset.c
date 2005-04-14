#include <combs.h>
#include <math.h>
#include <ugens.h>


static const float loopt[6] = {.0297,.0371,.0411,.0437,.005,.0017};
static const float alprvt[2] = {.096835,.032924};

/* storage reqs for above are .1583*SR+18 */

void
rvbset(float SR, float rvt, int init, float *a)
{
	float rvbtime,*apoint;
	int i;

	for(apoint=a, i=0; i<NCOMBS; i++) {
		rvbtime = (i<(NCOMBS-2)) ? rvt : alprvt[i-(NCOMBS-2)];
		combset(SR, loopt[i],rvbtime,init,apoint);
		apoint += (int)*apoint;
	}
}

