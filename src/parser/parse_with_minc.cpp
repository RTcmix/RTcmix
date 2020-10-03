/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdio.h>
#include <stdlib.h>
#include "minc/rename.h"
#include "minc/minc_defs.h"
#include "minc/bison_version.h"
#include "rtcmix_parse.h"
#include <ugens.h>
#include "rtdefs.h"
#include <Option.h>

typedef struct yy_buffer_state * YY_BUFFER_STATE;
#if BISON_VERSION >= 3
extern YY_BUFFER_STATE yy_scan_bytes(const char * buf, int len);
#else
extern YY_BUFFER_STATE yy_scan_bytes(const char * buf, size_t len);
#endif

extern "C" {
    extern int yyparse();
    extern int check_new_arg(const char *); /* Defined in minc/args.cpp */
    static int run_parser(const char *caller);
    int RTcmix_parseScore(char *thebuf, int buflen);
}

static int
run_parser(const char *caller)
{
    int status;
    try {
        status = yyparse();
    }
    catch (MincError err) {
        const char *errType = NULL;
        switch (err) {
            case MincInstrumentError:
                errType = "Instrument error";
                break;
            case MincParserError:
                errType = "Parser error";
                break;
            case MincSystemError:
                errType = "System error";
                break;
            case MincInternalError:
                errType = "Internal error";
                break;
        }
        rtcmix_warn(caller, "caught parse exception: %s", errType);
        status = err;
    }
    catch (int otherError) {
        const char *errname;
        switch (otherError) {
            case FUNCTION_NOT_FOUND:
                errname = "Function or instrument not found";
                break;
            case PARAM_ERROR:
                errname = "Illegal or missing parameter";
                break;
            case CONFIGURATION_ERROR:
                errname = "Configuration error";
                break;
            case AUDIO_ERROR:
                errname = "Audio device error";
                break;
            case FILE_ERROR:
                errname = "File error";
                break;
            case SYSTEM_ERROR:
                errname = "RTcmix system error";
                break;
            case RESOURCE_ERROR:
                errname = "Out of resources";
                break;
            case MEMORY_ERROR:
                errname = "Memory error";
                break;
            default:
                errname = "Other error";
        }
        rtcmix_warn(caller, "Caught exception: %s", errname);
        status = otherError;
    }
    if (status != 0) {
        // added for exit after Minc parse errors with the option set -- BGG
        if (get_bool_option(kOptionExitOnError)) {
            rtcmix_warn(caller, "Exit-on-error enabled - exiting");
            exit(1);
        }
        // If the parser returns a bad status, it must be torn down and recreated
        destroy_parser();
    }
    return status;
}

int parse_score_buffer(const char *buffer, int buflen)
{
    YY_BUFFER_STATE yybuf = yy_scan_bytes(buffer, buflen);
    reset_parser();
    preserveSymbols(true);
    return run_parser("parse_score_buffer");
}

#ifndef EMBEDDED

/* Defined in sys/command_line.c */
extern char *aargv[];
extern int aargc;

/* ---------------------------------------------------------- parse_score --- */
int
parse_score(int argc, char *argv[], char **env)
{
	int aarg = 0, status, new_arg_count = 0;
	
	/* Copy command-line args to make them available to the Minc-only
	 functions in sys/command_line.c: f_arg, i_arg, s_arg, and n_arg.
	 */
	for (int arg = 1; arg < argc; arg++) {
		// grab --arguments to store for use as tokens in Minc.  Otherwise store argument in aargv
		if (check_new_arg(argv[arg]) == 1) {
			++new_arg_count;
		}
		else {
			aargv[aarg++] = argv[arg];
		}
	}
	aargc = aarg;	// doesn't count args we pulled out above
	
    preserveSymbols(false);
	status = run_parser("parse_score");
	
	return status;
}

#else

// BGG mm -- set this to accept a buffer from max/msp
int RTcmix_parseScore(char *theBuf, int buflen)
{
    YY_BUFFER_STATE buffer = yy_scan_bytes(theBuf, buflen);
    reset_parser();
    preserveSymbols(true);
    return run_parser("RTcmix_parseScore");
}
    
#endif

/* ------------------------------------------------------ use_script_file --- */
/* Parse file <fname> instead of stdin. */
void
use_script_file(char *fname)
{
#ifndef EMBEDDED
	/* <yyin> is yacc's input file. If left alone, stdin will be used. */
	extern FILE *yyin;
	
	// BGG mm -- we don't use this in Max/MSP, and there is no yy_in var
	yyin = fopen(fname, "r+");
	if (yyin == NULL) {
		RTFPrintf(stderr, "Can't open %s\n", fname);
		exit(1);
	}
	if (get_print_option() > MMP_ADVISE)
		RTPrintf("Using file %s\n", fname);
#else
    rterror("use_script_file", "Command not available for embedded builds");
#endif
}

#ifdef EMBEDDED
extern "C" double minc_memflush();                                    // minc/minc.cpp (from minc.y)
#endif

/* ------------------------------------------------------- destroy_parser --- */
void
destroy_parser()
{
	yylex_destroy();
#ifdef EMBEDDED
	(void)minc_memflush();
	clear_node_state();
#endif
}
