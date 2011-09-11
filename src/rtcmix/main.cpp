/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#define DBUG
//#define DENORMAL_CHECK
#include <pthread.h>
#include <sys/resource.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "RTcmixMain.h"
#include "prototypes.h"
#include <ugens.h>
#include <ug_intro.h>
#include "version.h"
#include "rt.h"
#include "heap.h"
#include "sockdefs.h"
#include "notetags.h"           // contains defs for note-tagging
#include "dbug.h"


extern "C" {
#ifdef SGI
  void flush_all_underflows_to_zero();
#endif
#ifdef LINUX
  void sigfpe_handler(int sig);
#endif
}


/* ----------------------------------------------------- detect_denormals --- */
/* Unmask "denormalized operand" bit of the x86 FPU control word, so that
   any operations with denormalized numbers will raise a SIGFPE signal,
   and our handler will be called.  NOTE: This is for debugging only!
   This will not tell you how many denormal ops there are, so just because
   the exception is thrown doesn't mean there's a serious problem.  For
   more info, see: http://www.musicdsp.org/files/other001.txt.
*/
#ifdef LINUX
#ifdef DENORMAL_CHECK
static void
detect_denormals()
{
   #include <fpu_control.h>
   int cw = 0;
   _FPU_GETCW(cw);
   cw &= ~_FPU_MASK_DM;
   _FPU_SETCW(cw);
}
#endif /* DENORMAL_CHECK */
#endif /* LINUX */


/* ----------------------------------------------------------------- main --- */
int
main(int argc, char *argv[], char **env)
{
#ifdef LINUX
 #ifdef DENORMAL_CHECK
   detect_denormals();
 #endif
   signal(SIGFPE, sigfpe_handler);          /* Install signal handler */
#endif /* LINUX */
#ifdef SGI
   flush_all_underflows_to_zero();
#endif

   RTcmixMain app(argc, argv, env);
   app.run();

   return 0;
}


