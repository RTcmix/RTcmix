/* command_line.c */
/* to  return command line arguments */
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include "../Minc/ext.h"

double f_arg(float *p, short n_args)
{
	return (((int)p[0]) < aargc - 1) ? (atof(aargv[(int)p[0]])) : 0.0;
}

double i_arg(float *p, short n_args)
{
	return (((int)p[0]) < aargc - 1) ? (atoi(aargv[(int)p[0]])) : 0;
}

double s_arg(float *p,short n_args,double *pp)
{
	char *name;
	int i1 = 0;
	if(((int)pp[0]) < aargc - 1) {
		name = aargv[(int)pp[0]];
		i1 = (int) strsave(name);
	}
	return(i1);
}

double n_arg(float *p, short n_args)
{
	return(aargc);
}

