#include <math.h>
#include "../H/ugens.h"

gen5(gen)

register struct gen *gen;

{
	float c,amp2,amp1;
	int j,k,l,i = 0;

	amp2 = gen->pvals[0];
	for(k = 1; k < gen->nargs; k += 2) {
		amp1 = amp2;
		amp2 = gen->pvals[k+1];
		j = i + 1;
		gen->array[i] = amp1;
		c = (float) pow((amp2/amp1),(1./gen->pvals[k]));
		i = (j - 1) + gen->pvals[k];
		for(l = j; l < i; l++) {
			if(l < gen->size)
				gen->array[l] = gen->array[l-1] * c;
  			}
		}
	fnscl(gen);
}
