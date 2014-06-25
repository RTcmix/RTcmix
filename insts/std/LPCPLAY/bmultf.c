#include "lp.h"

void
bmultf(float *array, float mult, int number)
{
	int i;
	for(i=0; i<number; ++i)
		array[i] *= mult;
}
