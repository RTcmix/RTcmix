#include "../H/ugens.h"

gen18(gen)

register struct gen *gen;

{
        setline(gen->pvals,gen->nargs,gen->size,gen->array);
}
