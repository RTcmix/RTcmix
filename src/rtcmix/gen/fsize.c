#include "../H/ugens.h"
#include <stdio.h>

extern int ngens;       /*total of gens so far, initialized in main at 1*/
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
