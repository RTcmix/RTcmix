#include <stdio.h> 
#include "../H/ugens.h"

extern FILE *infile_desc[50];	/* contains file descriptors for data files */

gen3(gen)
    register struct gen *gen;
{
    double atof();
    float val; 
    int i = 0;
    FILE *in_desc;

    /* input datafile is stdin if pval[0] = 0 */
  
    in_desc = infile_desc[(int)gen->pvals[0]];

    if(in_desc == NULL) /* Stop if infile seek failed */ 
    { 
        fprintf(stderr,"Input file error. Gen03 exited.\n");
        return;
    } 
    /* read input file until EOF */

    while(fread(&val, sizeof(val), 1, in_desc)) {
	if(i >= gen->size)
		break;
	gen->array[i] = val;
	i++;
    }
    if(i > gen->size) fprintf(stderr,"Out of array space in gen03!!\n");
    
    printf("%d values loaded into array.\n", (i<=gen->size)?i:gen->size);

    i--;
    while(++i < gen->size)        /* fill remainder (if any) with zeros */
	gen->array[i] = 0.0;

    /* fnscl(gen); */ /* no rescaling of this gen */
}
