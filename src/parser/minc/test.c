#include <stdio.h>
#include <rtcmix_types.h>
#include "minc.h"

static void
init()
{
}

static void
final()
{
}

/* CAUTION: must match signature declared in parse_dispatch.h */
int
parse_dispatch(const char *funcname, const Arg arglist[], const int nargs,
   Arg *retval)
{
   retval->val.number = 0.0;
   retval->type = DoubleType;

   printf("parse_dispatch: n_args=%d\n", nargs);
   return 0;
}

int
main()
{
   init();
   yyparse();
   final();
   return 0;
}

