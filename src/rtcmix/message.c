/* message functions, designed for use by instruments and others.
                                                        -JGG, 5/16/00
*/
/* modified to allow die() to continue (i.e. no exit) if ERROR_ON_EXIT
   is undef'd in makefile.conf
   -- BGG 1/2004
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "globals.h"
#include <rtdefs.h>
#include "prototypes.h"
#include <ugens.h>
#include <Option.h>

#define PREFIX  "*** "       /* print before WARNING and ERROR */
#define BUFSIZE 1024

#ifdef SGI  /* not as safe, but what can ya do */
#define vsnprintf(str, sz, fmt, args)  vsprintf(str, fmt, args)
#endif

/* These functions are wrappers for printf that do a little extra work.
   The first arg is the name of the instrument they're called from.
   Non-instrument code can call these and pass NULL for <inst_name>.
   The remaining args are just like printf (format string and a variable
   number of arguments to fill it).  advise() and warn() take into account
   the current state of the print option, while die() prints no matter what,
   and then exits after some cleanup.
*/

/* --------------------------------------------------------------- advise --- */
void
advise(const char *inst_name, const char *format, ...)
{
   if (get_print_option()) {
      char     buf[BUFSIZE];
      va_list  args;

      va_start(args, format);
      vsnprintf(buf, BUFSIZE, format, args);
      va_end(args);

      if (inst_name)
         printf("%s:  %s\n", inst_name, buf);
      else
         printf("%s\n", buf);
   }
}


/* --------------------------------------------------------------- warn --- */
void
warn(const char *inst_name, const char *format, ...)
{
   if (get_print_option()) {
      char     buf[BUFSIZE];
      va_list  args;

      va_start(args, format);
      vsnprintf(buf, BUFSIZE, format, args);
      va_end(args);

      if (inst_name)
         fprintf(stderr, "\n" PREFIX "WARNING [%s]:  %s\n\n", inst_name, buf);
      else
         fprintf(stderr, "\n" PREFIX "WARNING:  %s\n\n", buf);
   }
}


/* -------------------------------------------------------------- rterror --- */
void
rterror(const char *inst_name, const char *format, ...)
{
   char     buf[BUFSIZE];
   va_list  args;

   va_start(args, format);
   vsnprintf(buf, BUFSIZE, format, args);
   va_end(args);

   if (inst_name)
      fprintf(stderr, PREFIX "ERROR [%s]: %s\n", inst_name, buf);
   else
      fprintf(stderr, PREFIX "ERROR: %s\n", buf);
}

/* ------------------------------------------------------------------ die --- */
int
die(const char *inst_name, const char *format, ...)
{
   char     buf[BUFSIZE];
   va_list  args;

   va_start(args, format);
   vsnprintf(buf, BUFSIZE, format, args);
   va_end(args);

   if (inst_name)
      fprintf(stderr, PREFIX "FATAL ERROR [%s]:  %s\n", inst_name, buf);
   else
      fprintf(stderr, PREFIX "FATAL ERROR:  %s\n", buf);

   if (get_bool_option(kOptionExitOnError)) {
      if (!rtsetparams_was_called())
         closesf_noexit();

      exit(1);
      return 0;	/*NOTREACHED*/
   }
   else
      return DONT_SCHEDULE;
}

