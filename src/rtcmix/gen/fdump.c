#include <ugens.h>
#include <stdio.h>

double
fdump(float p[], short n_args)
{
   int   genslot;
   float *array;

   genslot = p[0];
   array = floc(genslot);
   if (array) {
      int i, len = fsize(genslot);
      printf("Dumping gen slot %d...\n", genslot);
      for (i = 0; i < len; i++)
         printf("[%d] %.6f\n", i, array[i]);
   }
   else
      die(NULL, "You must make a gen before dumping it!");

   return 0.0;
}

