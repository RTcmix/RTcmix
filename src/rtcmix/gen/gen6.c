#include <math.h>
#include "../H/ugens.h"

double
gen6(struct gen *gen)
{
   setline(gen->pvals, gen->nargs, gen->size, gen->array);
   fnscl(gen);

   return 0.0;
}

