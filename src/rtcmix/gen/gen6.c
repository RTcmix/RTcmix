#include <math.h>
#include "../H/ugens.h"

gen6(gen)

register struct gen *gen;

{
	setline(gen->pvals,gen->nargs,gen->size,gen->array);
	fnscl(gen);
}
