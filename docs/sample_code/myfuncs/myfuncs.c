#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ugens.h>
  
#define DEBUG


/* ------------------------------------------------------------- somefunk --- */
double
somefunk(float p[], int n_args, double pp[])
{
   int   an_int_arg;
   float val, a_float_arg;

   if (n_args < 1) {
      fprintf(stderr, "somefunk: wrong number of args.\n");
      exit(1);
   }

   an_int_arg = (int) p[0];
   a_float_arg = p[0];

   val = a_float_arg * 2;

   return val;
}

/* -------------------------------------------------------------- profile --- */
void
profile()
{
   UG_INTRO("somefunk", somefunk);
}
