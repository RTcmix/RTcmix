/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdio.h>
#include <stdlib.h>
#include "rtcmix_parse.h"
#include <Option.h>

extern int yyparse();

/* <yyin> is yacc's input file. If we leave it alone, stdin will be used. */
extern FILE *yyin;

/* Defined in sys/command_line.c */
extern char *aargv[];
extern int aargc;


/* ---------------------------------------------------------- parse_score --- */
int
parse_score(int argc, char *argv[], char **env)
{
   int   i, status;

   /* Copy command-line args to make them available to the Minc-only
      functions in sys/command_line.c: f_arg, i_arg, s_arg, and n_arg.
   */
   for (i = 1; i < argc; i++)
      aargv[i - 1] = argv[i];
   aargc = argc - 1;

   status = yyparse();

   return status;
}


/* ------------------------------------------------------ use_script_file --- */
/* Parse file <fname> instead of stdin. */
void
use_script_file(char *fname)
{
   yyin = fopen(fname, "r+");
   if (yyin == NULL) {
      fprintf(stderr, "Can't open %s\n", fname);
      exit(1);
   }
   if (get_bool_option(kOptionPrint))
      printf("Using file %s\n", fname);
}


/* ------------------------------------------------------- destroy_parser --- */
void
destroy_parser()
{
   /* nothing to do for Minc */
}

