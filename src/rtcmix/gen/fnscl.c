#include "../H/ugens.h"
fnscl(gen)
register struct gen *gen;
{
	double fabs();
	int j;
	float wmax,xmax = 0;
	if(gen->slot < 0) return;
	for(j = 0; j < gen->size; j++) {
		if ((wmax = fabs(gen->array[j])) > xmax) xmax = wmax;
	}
	for(j = 0; j < gen->size; j++) {
		gen->array[j] /= xmax;
	}
}
