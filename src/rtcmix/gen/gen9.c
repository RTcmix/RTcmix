#include <math.h>
#include "../H/ugens.h"

double
gen9(struct gen *gen)
{
   int i, j;

   for (i = 0; i < gen->size; i++)
      gen->array[i] = 0;

   for (j = gen->nargs - 1; j > 0; j -= 3) {
      if (gen->pvals[j - 1] != 0) {
         for (i = 0; i < gen->size; i++) {
            double val = sin(PI2 * ((float) i / ((float) (gen->size)
                                 / gen->pvals[j - 2]) + gen->pvals[j] / 360.));
            gen->array[i] += (float) (val * gen->pvals[j - 1]);
         }
      }
   }
   fnscl(gen);

   return 0.0;
}

