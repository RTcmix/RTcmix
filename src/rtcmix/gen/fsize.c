#include <ugens.h>
#include <stdio.h>

/* these 3 defined in makegen.c */
extern float *farrays[];
extern int sizeof_farray[];
extern int f_goto[];


int
fsize(int genno)
/*   returns the size of function number genno */
{
	if(!sizeof_farray[f_goto[genno]]) {
	       fprintf(stderr,"fsize: You haven't allocated function %d yet!\n",genno);
		closesf();  
	}
	return(sizeof_farray[f_goto[genno]]);
}
