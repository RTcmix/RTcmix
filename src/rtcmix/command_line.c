/* command_line.c */
/* to  return command line arguments */
#include "../H/ugens.h"
#include "../Minc/defs.h"
#include "../Minc/ext.h"
extern int aargc;
extern char *aargv[];          /* to pass commandline args to subroutines */

double f_arg(float *p, short n_args)
{
	double atof();
	return (((int)p[0]) < aargc - 1) ? (atof(aargv[(int)p[0]])) : 0.0;
}

double i_arg(float *p, short n_args)
{
	int atoi();
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

