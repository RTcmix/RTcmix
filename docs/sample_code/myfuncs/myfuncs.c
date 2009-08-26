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
   char  *a_str_arg;

   if (n_args < 1)
      die("somefunk", "Wrong number of args.");

   an_int_arg = (int) p[0];
   a_float_arg = p[0];

   /* This is the method of getting a string pointer
      from Minc. Minc allocates space for the string that persists
      for the life of the program, so we just need a pointer to it.
   */
   a_str_arg = DOUBLE_TO_STRING(pp[0]);

   /* We'll just interpret the arg as a float here. */
   val = a_float_arg * 2;

   return val;
}


/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("somefunk", somefunk);
   return 0;
}

