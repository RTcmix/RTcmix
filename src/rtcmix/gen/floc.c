#include "../H/ugens.h"
#include <stdio.h>

extern int ngens;       /*total of gens so far, initialized in main at 1*/
extern float *farrays[];
extern int sizeof_farray[];
extern int f_goto[];


float *floc(int genno)
/*       returns the address of function number genno  */
{
#ifdef NOMORE
	if(!sizeof_farray[f_goto[genno]]) {
               fprintf(stderr,"floc: You haven't allocated function %d yet.\n",genno);
		closesf();
	}
	return(farrays[f_goto[genno]]);
#else
	if (sizeof_farray[f_goto[genno]] == 0)
		return NULL;
	else
		return farrays[f_goto[genno]];
#endif
}
