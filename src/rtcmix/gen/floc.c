#include "../H/ugens.h"
#include <stdio.h>

/* these 3 defined in makegen.c */
extern float *farrays[];
extern int sizeof_farray[];
extern int f_goto[];

/* Returns the address of function number genno.
   If genno is 1, then returns NULL if that function array doesn't exist.
   This is part of the new setline scheme:
      m_setline uses genno 1 in its call to makegen, and
      insts assume that genno 1 contains the overall amplitude envelope.
      If genno 1 doesn't exist, insts use 1.0 as an amp multiplier, rather
        than grabbing one out of a function table. (No more lineset flag.)
   If genno is not 1 and doesn't exist, complains and bails out.

   NOTE: Some insts don't use genno 1 as an amp envelope, so they need to
   check the return of floc and complain themselves if it's NULL.
*/

float *floc(int genno)
{
   int index = f_goto[genno];
   int size = sizeof_farray[index];

   if (size == 0) {
      if (genno == 1)
         return NULL;
      else {
         fprintf(stderr, "You haven't allocated function %d yet!\n", genno);
         closesf();
      }
   }
   else
      return farrays[index];
}

