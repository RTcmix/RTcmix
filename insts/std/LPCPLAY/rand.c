/* LOCAL COPIES OF BLOCK RAND CODE */
#include "lp.h"

static  long    l_randx = 1;

void l_srrand(unsigned x)
{
        l_randx = x;
}

//static int printum = 0;
void
l_brrand(float amp, float *a, int j)
{
	int i, k;
	for(k=0; k<j; k++) {
		i = ((l_randx = l_randx*1103515245 + 12345)>>16) & 077777;
		a[k] = amp * ((float)i/16384.0f - 1.0f);
//		if (printum) printf("a[%d] = %g, i = %d\n", k, a[k], i);
	}
}
