/* This file is no longer auto-generated -- do not erase or remove!! */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "../../H/maxdispargs.h"

extern double parse_dispatch(char *, double [], int, void **);

MODULE = RT     PACKAGE = RT

double
handle_minc_function(...)

   CODE:
      {
         char     *function_name;
         int      i;
         double   pp[MAXDISPARGS];
         SV       *stack_item;

         /* function name passed in as first argument */
         stack_item = ST(0);
         function_name = SvPV_nolen(stack_item);

         if (items - 1 > MAXDISPARGS)
            croak("Too many arguments to %s.", function_name);

         for (i = 1; i < items; i++) {
            stack_item = ST(i);

            /* NOTE: Check for double/integer first; otherwise you might
               get the string version of a number, which we'll then cast
               as a string pointer to RTcmix.
            */
            if (SvNIOK(stack_item)) {
               pp[i-1] = SvNV(stack_item);
            }
            else {
               char     *str;

# FIXME: may need to strdup <str>, since perl might garbage-collect it
               str = SvPV_nolen(stack_item);
               pp[i-1] = (double) ((int) str);
            }
         }

         RETVAL = parse_dispatch(function_name, pp, items - 1, NULL);
      }
   OUTPUT:
      RETVAL

