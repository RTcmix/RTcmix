/* RTcmix  - Copyright (C) 2001  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* Functions for managing embedded Python parser.    -JGG, 12-Feb-01 */

#include <assert.h>
#include <Python.h>
#include "../Python/rtcmix_python_ext.h"
#include "rtcmix_parse.h"

static FILE *script = NULL;


/* ---------------------------------------------------------- parse_score --- */
int
parse_score(int argc, char *argv[])
{
   int   i, status, xargc;
   char  *xargv[MAXARGS + 2];

   assert(argc <= MAXARGS);

   Py_SetProgramName(argv[0]);

   /* Init Python interpreter.  If this fails, it won't return. */
   Py_Initialize();

   /* Define sys.argv in Python. */
   PySys_SetArgv(argc, argv);

   /* Init our static module (../Python/rtcmix_python_ext.o). */
   status = init_rtcmix_python_ext();

   if (script == NULL)
      script = stdin;
   /* Otherwise, <script> will have been set by use_script_file. */

   /* Run the Python interpreter. */
   if (status == 0) {
      PyRun_AnyFile(script, "");    // FIXME: 2nd arg should be script name

      /* Kill interpreter, so that it won't trap cntl-C while insts play.
         Actually, it turns out that this doesn't help, at least for 
         Python 2.x, so we have to reinstall our SIGINT handler in main().
      */
      Py_Finalize();                /* any errors ignored internally */
   }
   else
      fprintf(stderr, "Can't create Python interpreter.\n");

   return status;
}


/* ------------------------------------------------------ use_script_file --- */
/* Parse file <fname> instead of stdin. */
void
use_script_file(char *fname)
{
   script = fopen(fname, "r+");
   if (script == NULL) {
      fprintf(stderr, "Can't open %s\n", fname);
      exit(1);
   }
   printf("Using file %s\n", fname);
}


/* ------------------------------------------------------- destroy_parser --- */
void
destroy_parser()
{
   /* nothing to do (see Py_Finalize() above) */
}

