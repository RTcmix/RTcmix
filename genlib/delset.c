#include "../H/ugens.h"

void
delset(float *a, int *l, float xmax)
{
/* delay initialization.  a is address of float array, l is size-2 int 
 * array for bookkeeping variables, xmax, is maximum expected delay */

	int i;
	*l = 0;
	*(l+1) = (int)(xmax * SR + .5);
	for(i = 0; i < *(l+1); i++) *(a+i) = 0;
}
