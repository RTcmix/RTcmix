#include <stdio.h> 
#include "../H/ugens.h"

extern FILE *infile_desc[50];	/* contains file descriptors for data files */

void gen2(register struct gen *gen)
{
    char inval[128], next;
    double atof();
    float val; 
    int i;
    FILE *in_desc;

    i=0;

    /* input datafile is stdin if pval[0] = 0 */
    if (gen->pvals[0] == 0)
      in_desc =  stdin;
    else
      in_desc = infile_desc[(int)gen->pvals[0]];

    if(in_desc == NULL) /* Stop if infile seek failed */ 
    { 
        fprintf(stderr, "Input error. Gen02 exited.\n");
        return;
    } 

    if(gen->pvals[0] == 0)            /* if reading from stdin */ 
    {
        while(fscanf(in_desc, "%f", &val) != EOF)
        {
            if(i < gen->size) gen->array[i] = val;
            i++;
            if(getc(in_desc) == 10) break;
        }
    }
    else   /* if reading from input text file specified with infile */
    {
        while(fscanf(in_desc, "%s", inval) != EOF) {
            if(i >= gen->size)
	      break;
	    gen->array[i] = atof(inval);
            i++;
        }
    } 
    if(i > gen->size) fprintf(stderr,"Out of array space in gen02!!\n");
    
    printf("%d values loaded into array.\n", (i<=gen->size)?i:gen->size);

    i--;
	while(++i < gen->size)        /* fill remainder (if any) with zeros */
		gen->array[i] = 0.0;

    /* fnscl(gen); */ /* no rescaling done for this gen */
}

