#include "../H/ugens.h"

gen10(gen)
register struct gen *gen;
{
	int i,j;
	double sin();
	for(i = 0; i<gen->size; i++) gen->array[i] = 0;
	j=gen->nargs;
	while(j--) {
		if(gen->pvals[j] != 0) {
			for(i=0; i<gen->size; i++) {
				gen->array[i]+=sin((double)(PI2*(float)i/
				(gen->size/(j+1))))*gen->pvals[j];
			}
		}
	}
	fnscl(gen);
}

