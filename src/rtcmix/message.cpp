/* message functions, designed for use by instruments and others.
                                                        -JGG, 5/16/00
*/
/* modified to allow die() to continue (i.e. no exit) if ERROR_ON_EXIT
   is undef'd in makefile.conf
   -- BGG 1/2004
*/
/* added capability to print into an internal buffer for 'imbedded' apps
	(#ifdef EMBEDDED's) and also to allow different printing levels with
	print_on(X).  See MMPrint.h for listing of levels
	-- BGG 1/2013
*/
/* printing levels are now in ugens.h -- DS 9/2013 */
/* added rtcmix_debug() to allow run-time debugging of important stuff -- DS 12/2013 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <globals.h>
#include <rtdefs.h>
#include <prototypes.h>
#include <ugens.h>
#include <Option.h>
#ifdef MAXMSP
// BGG -- this is how you print to the console.app now in max/msp
extern void cpost(const char *fmt, ...);
#define USE_POST
#elif defined(PD)
// DAS -- this is PD's version of same
extern void post(const char *fmt, ...);
#define cpost post
#define USE_POST
#endif

#if (defined(MACOSX) || defined(IOS)) && defined(EMBEDDED)
#include <syslog.h>
#define USE_SYSLOG
#endif


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

/* -------------------------------------------------------- rtcmix_debug --- */
void rtcmix_debug(const char *inst_name, const char *format, ...)
{
	if (get_print_option() >= MMP_DEBUG) {
		char     buf[BUFSIZE];
		va_list  args;
		
		va_start(args, format);
		vsnprintf(buf, BUFSIZE, format, args);
		va_end(args);
		
		if (inst_name) {
			RTPrintf("DEBUG: %s:  %s\n", inst_name, buf);
#ifdef USE_SYSLOG
			syslog(LOG_DEBUG, "DEBUG: %s:  %s", inst_name, buf);
#elif defined(USE_POST)
			cpost("DEBUG: [%s]:  %s", inst_name, buf);
#endif
		}
		else {
			RTPrintf("DEBUG: %s\n", buf);
#ifdef USE_SYSLOG
			syslog(LOG_DEBUG, "DEBUG: %s", buf);
#elif defined(USE_POST)
			cpost("DEBUG: %s", buf);
#endif
		}
	}
}

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

	   if (inst_name) {
			RTFPrintf(stderr, "\n" PREFIX "WARNING [%s]:  %s\n\n", inst_name, buf);
#ifdef USE_SYSLOG
			syslog(LOG_WARNING, PREFIX "WARNING: [%s]:  %s", inst_name, buf);
#elif defined(USE_POST)
		   cpost(PREFIX "WARNING: [%s]:  %s", inst_name, buf);
#endif
	   }
	   else {
			RTFPrintf(stderr, "\n" PREFIX "WARNING:  %s\n\n", buf);
#ifdef USE_SYSLOG
			syslog(LOG_ERR, PREFIX "WARNING: %s", buf);
#elif defined(USE_POST)
			cpost(PREFIX "WARNING: %s", buf);
#endif
	   }
   }
}

void rtcmix_print(const char *format, ...)
{
	char     buf[BUFSIZE];
	va_list  args;
	
	va_start(args, format);
	vsnprintf(buf, BUFSIZE, format, args);
	va_end(args);

#ifdef USE_SYSLOG
	syslog(LOG_NOTICE, "DEBUG: %s", buf);
#elif defined(USE_POST)
	cpost("DEBUG: %s", buf);
#else
	printf("DEBUG: %s", buf);
#endif
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

	if (inst_name) {
		RTFPrintf(stderr, PREFIX "ERROR [%s]: %s\n", inst_name, buf);
#ifdef USE_SYSLOG
#ifndef IOSDEV
		fprintf(stderr, PREFIX "ERROR: [%s]:  %s\n", inst_name, buf);
#endif
		syslog(LOG_ERR, PREFIX "ERROR: [%s]:  %s", inst_name, buf);
#elif defined(USE_POST)
		cpost(PREFIX "ERROR: [%s]:  %s", inst_name, buf);
#endif
	}
	else {
		RTFPrintf(stderr, PREFIX "ERROR: %s\n", buf);
#ifdef USE_SYSLOG
#ifndef IOSDEV
		fprintf(stderr, PREFIX "ERROR: %s\n", buf);
#endif
		syslog(LOG_ERR, PREFIX "ERROR: %s", buf);
#elif defined(USE_POST)
		cpost(PREFIX "ERROR: %s", buf);
#endif
	}
#if 0
// allow embedded system to bail out of parser on all errorss
    if (get_bool_option(kOptionBailOnError)) {
        if (!rtsetparams_was_called())
            closesf_noexit();
        throw(SYSTEM_ERROR);
    }
#endif
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

	if (inst_name) {
		RTFPrintf(stderr, PREFIX "FATAL ERROR [%s]:  %s\n", inst_name, buf);
#ifdef USE_SYSLOG
#ifndef IOSDEV
		fprintf(stderr, PREFIX "FATAL ERROR: [%s]:  %s\n", inst_name, buf);
#endif
		syslog(LOG_ERR, PREFIX "FATAL ERROR: [%s]:  %s", inst_name, buf);
#elif defined(USE_POST)
		cpost(PREFIX "FATAL ERROR: [%s]:  %s", inst_name, buf);
#endif
	}
	else {
		RTFPrintf(stderr, PREFIX "FATAL ERROR:  %s\n", buf);
#ifdef USE_SYSLOG
#ifndef IOSDEV
		fprintf(stderr, PREFIX "FATAL ERROR:  %s\n", buf);
#endif
		syslog(LOG_ERR, PREFIX "FATAL ERROR:  %s", buf);
#elif defined(USE_POST)
		cpost(PREFIX "FATAL ERROR: %s", buf);
#endif

	}
    return DONT_SCHEDULE;
}

RTCmixStatus rtOptionalThrow(RTCmixStatus status)
{
    if (get_bool_option(kOptionBailOnError)) {
        throw(status);
    }
    return status;
}
