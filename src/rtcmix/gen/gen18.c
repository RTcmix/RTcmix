#include "../H/ugens.h"

double
gen18(struct gen *gen)
{
   setline(gen->pvals, gen->nargs, gen->size, gen->array);
   return 0.0;
}

