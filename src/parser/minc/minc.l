/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
%s INIT IGNORE COM CPCOM
%x INCLUDE
%{
#include <list>
#include "rename.h"
#include "minc_internal.h"
#include "minc.hpp"
#define YY_ALWAYS_INTERACTIVE 1
#define DEBUG_INCLUDES 0
static int comments = 0;
static int cpcomments = 0;
extern void yyerror(const char *);
extern void yyfatalerror(const char *);
/* To allow includes of subfiles */
#define MAX_INCLUDE_DEPTH 10
struct Include { int saved_lineno; YY_BUFFER_STATE state; };
static Include include_stack[MAX_INCLUDE_DEPTH];
static int include_stack_index = 0;
std::list<const char *> includedFilenames;
#define YY_FATAL_ERROR(msg) yyfatalerror( msg )
#ifdef EMBEDDED
#ifdef __cplusplus
extern "C" {
#endif
#ifdef yy_flex_strncpy
/* Because of the peculiarities of Lex, this function does not get declared or defined if the token 'yytext_ptr'
 is #defined.  Because we redefine all non-static yy* symbols for embedded builds, we have to define our own version
 of this method here.
 */
static void yy_flex_strncpy (char* s1, yyconst char * s2, int n)
{
	int i;
	for ( i = 0; i < n; ++i )
	s1[i] = s2[i];
}
#endif
#ifdef __cplusplus
}
#endif
#endif
%}

let   [a-zA-Z]
dig   [1234567890]
real  ({dig}+)((".")({dig}*)(("E"|"e")(("+"|"-")?)({dig}+))?)?
real2 (".")({dig}+)(("E"|"e")(("+"|"-")?)({dig}+))?
num   {dig}+

%%
                BEGIN(INIT);
<INIT>"/*"     { BEGIN COM; comments++; }
<INIT>"//"     { BEGIN CPCOM; cpcomments = 1; }
<INIT>#        { BEGIN IGNORE; }
<IGNORE>\n     { BEGIN INIT; }
<IGNORE>.      ;
<COM>"/*"      { comments++; }
<COM>.|\n      ;
<COM>"*/"      { comments--; if (comments == 0 && !cpcomments) { BEGIN INIT; } }
<CPCOM>\n      { cpcomments = 0; if (comments == 0) { BEGIN INIT; } }
<CPCOM>.    ;
<INIT>{real}            return TOK_NUM;
<INIT>{real2}           return TOK_NUM;
<INIT>"<>"|"!="         return TOK_UNEQU;
<INIT>"<="              return TOK_LESSEQU;
<INIT>">="              return TOK_GTREQU;
<INIT>"=="              return TOK_EQU;
<INIT>"+="              return TOK_PLUSEQU;
<INIT>"-="              return TOK_MINUSEQU;
<INIT>"*="              return TOK_MULEQU;
<INIT>"/="              return TOK_DIVEQU;
<INIT>"++"              return TOK_PLUSPLUS;
<INIT>"--"              return TOK_MINUSMINUS;
<INIT>"**"|"^"          return TOK_POW;
<INIT>"\""[^\"]*"\""    return TOK_STRING;
<INIT>"\'"[^\']*"\'"    return TOK_STRING;
<INIT>for               return TOK_FOR;
<INIT>if                return TOK_IF;
<INIT>else              return TOK_ELSE;
<INIT>while             return TOK_WHILE;
<INIT>and               return TOK_AND;
<INIT>"&&"              return TOK_AND;
<INIT>not               return TOK_NOT;
<INIT>"!"|"~"           return TOK_NOT;
<INIT>or                return TOK_OR;
<INIT>"||"              return TOK_OR;
<INIT>true              return TOK_TRUE;
<INIT>TRUE              return TOK_TRUE;
<INIT>false             return TOK_FALSE;
<INIT>FALSE             return TOK_FALSE;
<INIT>struct            return TOK_STRUCT_DECL;
<INIT>float             return TOK_FLOAT_DECL;
<INIT>string            return TOK_STRING_DECL;
<INIT>handle            return TOK_HANDLE_DECL;
<INIT>list            	return TOK_LIST_DECL;
<INIT>return			return TOK_RETURN;
<INIT>[%={}><,.;*+-]     return (*yytext);
<INIT>"["|"]"|"/"       return (*yytext);
<INIT>"("|")"           return (*yytext);
<INIT>"\n"        ;
<INIT>" "|"\t"       ;
<INIT>^include		  	{ BEGIN INCLUDE; }
<INIT>({let}|"_")({let}|{dig}|"#"|"_")*   return TOK_IDENT;
<INIT>"?"({let}|"_")({let}|{dig}|"_")*   return TOK_ARG_QUERY;
<INIT>"$"({let}|"_")({let}|{dig}|"_")*   return TOK_ARG;
<INIT>.                 { char msg[32]; snprintf(msg, 32, "invalid character: (%u) '%c'", (unsigned)yytext[0], yytext[0]); yyerror(msg); }
^include		  		{ BEGIN INCLUDE; }
<INCLUDE>[ \t]*      /* eat the whitespace */
<INCLUDE>[^ \t\n]+  { /* got the include file name */
            const char *includedFilename = yytext;
#if DEBUG_INCLUDES
            printf("including '%s'\n", includedFilename);
#endif
			if (include_stack_index >= MAX_INCLUDE_DEPTH) {
				YY_FATAL_ERROR( "Includes nested too deeply");
			}
            // Check whether this file was already included at some earlier level.
            bool found = false;
            for (std::list<const char *>::const_iterator it = includedFilenames.begin(); it != includedFilenames.end(); ++it) {
                if (strcmp(includedFilename, *it) == 0) {
                    found = true;
#if DEBUG_INCLUDES
                    printf("'%s' already included once - skipping\n", includedFilename);
#endif
                    break;
                }
            }
            if (!found) {
                includedFilenames.push_back(strdup(includedFilename));
                include_stack[include_stack_index].saved_lineno = yyget_lineno();
                include_stack[include_stack_index++].state = YY_CURRENT_BUFFER;
                yyin = fopen( yytext, "r" );
                if (!yyin)
                {
                    char msg[1024];
                    snprintf(msg, 1024, "Unable to open include file \"%s\"",
                             yytext);
                    YY_FATAL_ERROR(msg);
                }
                yyset_lineno(1);
                yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
            }
            BEGIN INIT;
		}

<<EOF>> {	if (--include_stack_index < 0)
			{
				yyterminate();
			}
			else {
				yy_delete_buffer(YY_CURRENT_BUFFER);
				yy_switch_to_buffer(include_stack[include_stack_index].state);
				increment_score_line_offset(yyget_lineno() - include_stack[include_stack_index].saved_lineno);
				yyset_lineno(include_stack[include_stack_index].saved_lineno);
			}
		} 
%%

