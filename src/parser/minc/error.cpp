/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "rename.h"
#include "minc_internal.h"
#include "minc_defs.h"

#define BUFSIZE 1024

#ifndef vsnprintf
#define vsnprintf(str, sz, fmt, args)  vsprintf(str, fmt, args)
#endif

static int exit_on_die = 0;

#ifdef EMBEDDED
int rtcmix_error = 0;		// referenced in tree.c
#endif

// BGG -- these in message.c.  Maybe just #include <ugens.h>?
extern void rtcmix_advise(const char *inst_name, const char *format, ...);
extern void rtcmix_warn(const char *inst_name, const char *format, ...);
extern void rterror(const char *inst_name, const char *format, ...);
extern int die(const char *inst_name, const char *format, ...);

void
sys_error(const char *msg)
{
	die("parser", "%s\n", msg);

	set_rtcmix_error(-1);

	if (exit_on_die)
      exit(EXIT_FAILURE);
}

int
configure_minc_error_handler(int exit)
{
   exit_on_die = exit;
   return 0;
}

void
minc_advise(const char *msg, ...)
{
   char buf[BUFSIZE];
   va_list args;

   va_start(args, msg);
   vsnprintf(buf, BUFSIZE, msg, args);
   va_end(args);

	rtcmix_advise("parser", buf);
}

void
minc_warn(const char *msg, ...)
{
   char buf[BUFSIZE];
   va_list args;

   va_start(args, msg);
   vsnprintf(buf, BUFSIZE, msg, args);
   va_end(args);

	rtcmix_warn("parser", "%s (near line %d)\n", buf, yyget_lineno());
}

void
minc_die(const char *msg, ...)
{
   char buf[BUFSIZE];
   va_list args;

   va_start(args, msg);
   vsnprintf(buf, BUFSIZE, msg, args);
   va_end(args);

	set_rtcmix_error(-1);

	rterror("parser", "%s (near line %d)\n", buf, yyget_lineno());
	

   if (exit_on_die)
      exit(EXIT_FAILURE);
}

void
minc_internal_error(const char *msg, ...)
{
   char buf[BUFSIZE];
   va_list args;

   va_start(args, msg);
   vsnprintf(buf, BUFSIZE, msg, args);
   va_end(args);

	set_rtcmix_error(-1);

	rterror("parser-program", "%s (near line %d)\n", buf, yyget_lineno());

	if (exit_on_die)
      exit(EXIT_FAILURE);
}

void
yyerror(const char *msg)
{
	rterror("parser-yyerror", "near line %d: %s\n", yyget_lineno(), msg);
}

#ifdef EMBEDDED
void set_rtcmix_error(int err)
{
	rtcmix_error = err;
}

Bool was_rtcmix_error()
{
	return (rtcmix_error != 0);
}
#endif