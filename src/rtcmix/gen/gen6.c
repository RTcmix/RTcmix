#include <math.h>
#include <ugens.h>

double
gen6(struct gen *gen)
{
   setline(gen->pvals, gen->nargs, gen->size, gen->array);
   fnscl(gen);

   return 0.0;
}

