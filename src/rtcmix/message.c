/* message functions, designed for use by instruments and others.
                                                        -JGG, 5/16/00
*/
#include <stdio.h>
#include <stdarg.h>
#include <globals.h>
#include <prototypes.h>
#include <ugens.h>

#define PREFIX  "*** "       /* print before WARNING and ERROR */
#define BUFSIZE 1024

// FIXME: do we need this on sgi?
//#ifdef SGI
//#define vsnprintf(str, sz, fmt, args)  vsprintf(str, fmt, args)
//#endif

/* These functions are wrappers for printf that do a little extra work.
   The first arg is the name of the instrument they're called from.
   Non-instrument code can call these and pass NULL for <inst_name>.
   The remaining args are just like printf (format string and a variable
   number of arguments to fill it).  advise() and warn() take into account
   the current state of <print_is_on>, while die() prints no matter what,
   and then exits after some cleanup.
*/

/* --------------------------------------------------------------- advise --- */
void
advise(const char *inst_name, const char *format, ...)
{
   if (print_is_on) {
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


/* ----------------------------------------------------------------- warn --- */
void
warn(const char *inst_name, const char *format, ...)
{
   if (print_is_on) {
      char     buf[BUFSIZE];
      va_list  args;

      va_start(args, format);
      vsnprintf(buf, BUFSIZE, format, args);
      va_end(args);

      if (inst_name)
         fprintf(stderr, PREFIX "WARNING [%s]:  %s\n", inst_name, buf);
      else
         fprintf(stderr, PREFIX "WARNING:  %s\n", buf);
   }
}


/* ------------------------------------------------------------------ die --- */
void
die(const char *inst_name, const char *format, ...)
{
   char     buf[BUFSIZE];
   va_list  args;

   va_start(args, format);
   vsnprintf(buf, BUFSIZE, format, args);
   va_end(args);

   if (inst_name)
      fprintf(stderr, PREFIX "ERROR [%s]:  %s\n", inst_name, buf);
   else
      fprintf(stderr, PREFIX "ERROR:  %s\n", buf);

   if (rtsetparams_called)
      rtcloseout();
   else
      closesf();

   exit(1);
}

