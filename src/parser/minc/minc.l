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
struct Include { int saved_lineno; YY_BUFFER_STATE state; FILE *file; const char *fileName; };
static Include include_stack[MAX_INCLUDE_DEPTH];
static int saved_lineno;
static int include_stack_index = 0;
/* Used to avoid duplicate includes */
std::list<const char *> includedFilenames;
static const char * current_include_file = NULL;
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
#endif  /* yy_flex_strncpy */
#ifdef __cplusplus
}
#endif
#endif  /* EMBEDDED */
extern "C" {

void yy_clear_includes()
{
    includedFilenames.clear();
    include_stack_index = 0;
    current_include_file = NULL;
    saved_lineno = 0;
}

void yy_store_lineno(int cur_lineno)
{
    saved_lineno = cur_lineno;
}

int yy_get_stored_lineno()
{
    return saved_lineno;
}

/* returns include index based on score parse */
int yy_get_include_index()
{
    return include_stack_index-1;
}

/* stores the include file associated with a score operation.  Used for error message display. */
void yy_set_current_include_filename(const char *include_file)
{
    current_include_file = include_file;
}

/* retrieves the include filename for a score operation.  Used for error message display. */
const char * yy_get_current_include_filename()
{
    return current_include_file;
}

}

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
<INIT>structbase        return TOK_BASE_DECL;
<INIT>float             return TOK_FLOAT_DECL;
<INIT>string            return TOK_STRING_DECL;
<INIT>handle            return TOK_HANDLE_DECL;
<INIT>list            	return TOK_LIST_DECL;
<INIT>map               return TOK_MAP_DECL;
<INIT>mfunction         return TOK_MFUNC_DECL;
<INIT>return		    return TOK_RETURN;
<INIT>method		    return TOK_METHOD;
<INIT>[%={}><,.;*+-]    return (*yytext);
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
            printf("\nincluding '%s' at line %d - depth %d\n\n", includedFilename, yyget_lineno(), include_stack_index+1);
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
                yyin = fopen(includedFilename, "r" );
#ifdef MINCLUDEDIR
                if (!yyin) {
                    // Did not find it locally - check the master MinC include dir if it is defined
                    char alternate_location[1054];
                    snprintf(alternate_location, 1054, "%s/%s", MINCLUDEDIR, includedFilename);
                    yyin = fopen(alternate_location, "r" );
                }
#endif
                if (!yyin) {
                    char msg[1054];
                    snprintf(msg, 1054, "Unable to open include file \"%s\"", includedFilename);
                    YY_FATAL_ERROR(msg);
                }
                else {
                    const char *fileName = strdup(includedFilename);
                    includedFilenames.push_back(fileName);
                    include_stack[include_stack_index].saved_lineno = yyget_lineno();
                    include_stack[include_stack_index].file = yyin;
                    include_stack[include_stack_index].fileName = fileName;
                    include_stack[include_stack_index++].state = YY_CURRENT_BUFFER;
                    current_include_file = fileName;
#if DEBUG_INCLUDES
                   printf("current_include_file set to '%s'\n", current_include_file);
                   printf("include stack:\n");
                    for (int idx = include_stack_index-1; idx >= 0; --idx) {
                        printf("%s\n", include_stack[idx].fileName);
                    }
#endif
                    yyset_lineno(1);
                    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
                }
            }
            BEGIN INIT;
		}

<<EOF>> {
#if DEBUG_INCLUDES
        if (include_stack_index > 0) {
            printf("\nPopping out of a depth-%d include\n", include_stack_index);
        }
#endif
        if (--include_stack_index < 0)
        {
            yyterminate();
        }
        else {
            yy_delete_buffer(YY_CURRENT_BUFFER);
            yy_switch_to_buffer(include_stack[include_stack_index].state);
            fclose(include_stack[include_stack_index].file);
            include_stack[include_stack_index].file = NULL;
            yyset_lineno(include_stack[include_stack_index].saved_lineno);
            current_include_file = (include_stack_index > 0) ? include_stack[include_stack_index-1].fileName : NULL;      // reset to popped level
            saved_lineno = 0;
#if DEBUG_INCLUDES
            printf("current_include_file reset to '%s'\n", current_include_file);
            printf("include stack:\n");
            for (int idx = include_stack_index-1; idx >= 0; --idx) {
                printf("%s\n", include_stack[idx].fileName);
            }
            printf("yylineno reset to %d\n\n", include_stack[include_stack_index].saved_lineno);
#endif
        }
    }
%%

