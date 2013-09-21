/* message functions, designed for use by instruments and others.
                                                        -JGG, 5/16/00
*/
/* modified to allow die() to continue (i.e. no exit) if ERROR_ON_EXIT
   is undef'd in makefile.conf
   -- BGG 1/2004
*/
/* added capability to print into an internal buffer for 'imbedded' apps
	(#ifdef MAXMSP's) and also to allow different printing levels with
	print_on(X).  See MMPrint.h for listing of levels
	-- BGG 1/2013
*/
/* printing levels are now in ugens.h -- DS 9/2013 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <globals.h>
#include <rtdefs.h>
#include <prototypes.h>
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
   number of arguments to fill it).  rtcmix_advise() and rtcmix_warn()
   take into account the current state of the print option, while die()
   prints no matter what, and then exits after some cleanup.
*/

/* -------------------------------------------------------- rtcmix_advise --- */
void
rtcmix_advise(const char *inst_name, const char *format, ...)
{
   if (get_print_option() >= MMP_ADVISE) {
      char     buf[BUFSIZE];
      va_list  args;

      va_start(args, format);
      vsnprintf(buf, BUFSIZE, format, args);
      va_end(args);

      if (inst_name)
         RTPrintf("%s:  %s\n", inst_name, buf);
      else
         RTPrintf("%s\n", buf);
   }
}


/* -------------------------------------------------------- rtcmix_warn --- */
void
rtcmix_warn(const char *inst_name, const char *format, ...)
{
   if (get_print_option() >= MMP_WARN) {
      char     buf[BUFSIZE];
      va_list  args;

      va_start(args, format);
      vsnprintf(buf, BUFSIZE, format, args);
      va_end(args);

      if (inst_name)
         RTFPrintf(stderr, "\n" PREFIX "WARNING [%s]:  %s\n\n", inst_name, buf);
      else
         RTFPrintf(stderr, "\n" PREFIX "WARNING:  %s\n\n", buf);
   }
}


/* -------------------------------------------------------------- rterror --- */
void
rterror(const char *inst_name, const char *format, ...)
{
   char     buf[BUFSIZE];
   va_list  args;

	if (get_print_option() < MMP_RTERRORS) return;

   va_start(args, format);
   vsnprintf(buf, BUFSIZE, format, args);
   va_end(args);

   if (inst_name)
      RTFPrintf(stderr, PREFIX "ERROR [%s]: %s\n", inst_name, buf);
   else
      RTFPrintf(stderr, PREFIX "ERROR: %s\n", buf);

// added for exit after Minc parse errors with the option set -- BGG
   if (get_bool_option(kOptionExitOnError)) {
      if (!rtsetparams_was_called())
         closesf_noexit();
      exit(1);
   }
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
      RTFPrintf(stderr, PREFIX "FATAL ERROR [%s]:  %s\n", inst_name, buf);
   else
      RTFPrintf(stderr, PREFIX "FATAL ERROR:  %s\n", buf);

   if (get_bool_option(kOptionExitOnError)) {
      if (!rtsetparams_was_called())
         closesf_noexit();
      exit(1);
      return 0;	/*NOTREACHED*/
   }
   else
      return DONT_SCHEDULE;
}

