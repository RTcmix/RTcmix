/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _MINC_H_
#define _MINC_H_ 1

/* Anything here is availale for export outside of the Minc directory. */

typedef enum _MincError
{
	MincInstrumentError		=	-1,		// call to instrument or internal routing failed
	MincParserError			=	-2,		// mistake in grammar
	MincSystemError			= 	-3,		// memory error, etc.
	MincInternalError		=	-4		// something wrong internally
} MincError;

int yyparse(void);
int yylex_destroy(void);

void reset_parser();
int configure_minc_error_handler(int exit);

#endif /* _MINC_H_ */
