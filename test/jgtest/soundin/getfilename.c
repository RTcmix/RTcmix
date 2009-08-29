#include <stdlib.h>
#include <ugens.h>
#include "getfilename.h"

static char *filename = NULL;


/* --------------------------------------------------------- get_filename --- */
char *
get_filename()
{
   return filename;
}


/* ------------------------------------------------------------- openfile --- */
/* Doesn't actually open the file; just gets filename from Minc.
*/
double
openfile(float p[], int n_args, double pp[])
{
   filename = DOUBLE_TO_STRING(pp[0]);

   if (filename[0] == '\0')      /* user passed "" from Minc */
      filename = NULL;

   return 0.0;
}


/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("openfile", openfile);
   return 0;
}

