/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdio.h>
#include <stdlib.h>
#include "minc/rename.h"
#include "minc/minc.h"
#include "rtcmix_parse.h"
#include <ugens.h>
#include <Option.h>

extern int yyparse();

#ifdef EMBEDDED

int RTcmix_parseScore(char *thebuf, int buflen);
extern int yyparse();
extern int yylineno;
extern void setGlobalBuffer(const char *inBuf, int inBufSize);

// BGG mm -- set this to accept a buffer from max/msp
int RTcmix_parseScore(char *theBuf, int buflen)
{
	// BGG -- added to reset the line # every time a new score buffer is received
	yylineno = 1;
	setGlobalBuffer(theBuf, buflen+1);
	return yyparse();
}

#else	// !EMBEDDED

/* <yyin> is yacc's input file. If left alone, stdin will be used. */
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

#endif	// !EMBEDDEDs

/* ------------------------------------------------------ use_script_file --- */
/* Parse file <fname> instead of stdin. */
void
use_script_file(char *fname)
{
#ifndef EMBEDDED
	// BGG mm -- we don't use this in Max/MSP, and there is no yy_in var
	yyin = fopen(fname, "r+");
	if (yyin == NULL) {
		RTFPrintf(stderr, "Can't open %s\n", fname);
		exit(1);
	}
#endif
	if (get_print_option() > MMP_ADVISE)
		RTPrintf("Using file %s\n", fname);
}


/* ------------------------------------------------------- destroy_parser --- */
void
destroy_parser()
{
	yylex_destroy();
}

