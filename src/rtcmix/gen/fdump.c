#include <ugens.h>
#include <stdio.h>

/* Send the contents of a makegen array to standard out or to an ASCII file.

   fdump(gennum [, "filename"])

   JGG, 2/13/02
*/
double
fdump(double p[], short n_args)
{
   int   genslot;
   double *array;
   FILE  *f = NULL;

   genslot = p[0];
   if (n_args > 1) {
      char *fname = DOUBLE_TO_STRING(p[1]);
      f = fopen(fname, "w+");
      if (f == NULL) {
         perror("fdump");
         return -1.0;
      }
   }
   else
      f = stdout;

   array = floc(genslot);
   if (array) {
      int i, len = fsize(genslot);
      RTPrintf("Dumping function table %d...\n", genslot);
      for (i = 0; i < len; i++)
         RTFPrintf(f, "%d %.6f\n", i, array[i]);
   }
   else
      die(NULL, "You must make a gen before dumping it!");

   if (f != stdout)
      fclose(f);

   return 0.0;
}

