/* infile.c -- datafile name-setting command for use with gen1. D.A.S. 9/89
*/

#include "../H/ugens.h"
#include "../H/sfheader.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE *infile_desc[50];


double
m_infile(p,n_args,pp) 
    float *p; 
    short n_args;
	double *pp;

{
    FILE *descrip;
    char  *name;
    int   fno,i;

    i = (int) pp[0];
    name = (char *) i;
    fno = p[1];

    descrip = fopen(name,"r");
    if(descrip == NULL) fprintf(stderr,"Cannot find %s...not opened.\n",name);
    else 
    {
	infile_desc[fno] = descrip;
	printf("Datafile %s opened as file %d\n", name, fno);
    }
    return fno;
}

