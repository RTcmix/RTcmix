/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
%{
#include <string.h>
#include <assert.h>
#include "rename.h"
#include "minc_internal.h"
#include "lex.yy.c"
#ifdef EMBEDDED
/* both in utils.c */
extern int readFromGlobalBuffer(char *buf, yy_size_t *pBytes, int maxbytes);
#endif
#define YYDEBUG 1
#define MAXTOK_IDENTLIST 200
#define TRUE 1
#define FALSE 0

#define MDEBUG	/* turns on yacc debugging below */

#ifdef MDEBUG
#define MPRINT(x) rtcmix_print("YACC: %s\n", x)
#define MPRINT1(x,y) rtcmix_print("YACC: " x "\n", y)
#define MPRINT2(x,y,z) rtcmix_print("YACC: " x "\n", y, z)
#else
#define MPRINT(x)
#define MPRINT1(x,y)
#define MPRINT2(x,y,z)
#endif

static Tree		program;
static Symbol	*sym;
static int		idcount = 0;	
static char		*idlist[MAXTOK_IDENTLIST];  
static int		flerror;		/* set if there was an error during parsing */
static int		level = 0;	/* keeps track whether we are in a structure */
static int		flevel = 0;	/* > 0 if we are in a function decl block */
static MincDataType funReturnType = MincVoidType;	/* ret type for function */
static char *	functionName;
static void 	cleanup();
static Tree declare(MincDataType type);
static struct symbol * declareFunction(const char *name, MincDataType returnType);
static Tree go(Tree t1);

%}

%left  <ival> LOWPRIO
%left  <ival> '='
%left  <ival> TOK_MINUSEQU
%left  <ival> TOK_PLUSEQU
%left  <ival> TOK_DIVEQU
%left  <ival> TOK_MULEQU
%left  <ival> TOK_OR
%left  <ival> TOK_AND
%left  <ival> TOK_EQU TOK_UNEQU
%left  <ival> '<' '>' TOK_LESSEQU TOK_GTREQU
%left  <ival> '+' '-'
%left  <ival> '*' '/'
%left  <ival> TOK_POW
%left  <ival> CASTTOKEN
%token <ival> TOK_FLOAT_DECL
%token <ival> TOK_STRING_DECL
%token <ival> TOK_HANDLE_DECL
%token <ival> TOK_IDENT TOK_NUM TOK_NOT TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_RETURN
%token <ival> TOK_TRUE TOK_FALSE TOK_STRING 
%type  <trees> stml stmt rstmt bexp expl exp str arg argl fargl fdef ret fstml bstml
%type  <str> id fname

%%
/* program (the "start symbol") */
prg:	| stml				{ MPRINT("prg:"); program = $1; cleanup(); return 0; }
	;
 
/* statement list */
stml:	stmt				{ MPRINT("<stmt>"); $$ = $1; }
	| stmt ';'				{ MPRINT("<stmt;>"); $$ = $1; }
	| stml stmt				{ MPRINT("<stml stmt>"); $$ = tseq($1, $2); }
	| stml stmt ';'			{ MPRINT("<stml stmt;>"); $$ = tseq($1, $2); }
	;

/* statement */
stmt: rstmt					{ MPRINT("<rstmt>");
								if (level == 0) 
									$$ = go($1); 
								else
									$$ = $1;
							}
	| TOK_FLOAT_DECL idl	{ $$ = declare(MincFloatType); idcount = 0; }	// e.g., "float x, y z"
	| TOK_STRING_DECL idl	{ $$ = declare(MincStringType); idcount = 0; }
	| TOK_HANDLE_DECL idl	{ $$ = declare(MincHandleType); idcount = 0; }
	/* N.B. The reason we have so many versions is that we do not want to treat <bstml> as a <stmt> */
	| TOK_IF level bexp stmt {
								level--; MPRINT1("level => %d", level);
								$$ = go(tif($3, $4));
							}
	| TOK_IF bexp bstml {
								level--; MPRINT1("level => %d", level);
								$$ = go(tif($2, $3));
							}
	| TOK_IF level bexp stmt TOK_ELSE stmt {
								level--; MPRINT1("level => %d", level);
								$$ = go(tifelse($3, $4, $6));
							}
	| TOK_IF bexp bstml TOK_ELSE stmt {
								level--; MPRINT1("level => %d", level);
								$$ = go(tifelse($2, $3, $5));
							}
	| TOK_IF level bexp stmt TOK_ELSE bstml {
								level--; MPRINT1("level => %d", level);
								$$ = go(tifelse($3, $4, $6));
							}
	| TOK_IF bexp bstml TOK_ELSE bstml {
								level--; MPRINT1("level => %d", level);
								$$ = go(tifelse($2, $3, $5));
							}
	| TOK_WHILE level bexp stmt	{
								level--; MPRINT1("level => %d", level);
								$$ = go(twhile($3, $4));
							}
	| TOK_WHILE bexp bstml	{
								level--; MPRINT1("level => %d", level);
								$$ = go(twhile($2, $3));
							}
	| TOK_FOR level '(' stmt ';' bexp ';' stmt ')' stmt {
								level--; MPRINT1("level => %d", level);
								$$ = go(tfor($4, $6, $8, $10));
							}
	| TOK_FOR '(' stmt ';' bexp ';' stmt ')' bstml {
								level--; MPRINT1("level => %d", level);
								$$ = go(tfor($3, $5, $7, $9));
							}
	| bstml						{
								level--; MPRINT1("level => %d", level);
								$$ = go($1);	/* standalone block gets executed immediately */
							}
	| fdef
	| error TOK_FLOAT_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_STRING_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_HANDLE_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_IF		{ flerror = 1; $$ = tnoop(); }
	| error TOK_WHILE	{ flerror = 1; $$ = tnoop(); }
	| error TOK_FOR	{ flerror = 1; $$ = tnoop(); }
	| error '{'			{ flerror = 1; $$ = tnoop(); }
	| error TOK_ELSE	{ flerror = 1; $$ = tnoop(); }
	| error TOK_RETURN	{ flerror = 1; $$ = tnoop(); }
	| error ';'			{ flerror = 1; $$ = tnoop(); }
	;

/* block of statements */

bstml:	'{' level stml '}'	{ MPRINT("<{stml}>"); $$ = tblock($3); }
	;

/* statement nesting level counter */
level:  /* nothing */ { level++; MPRINT1("level => %d", level); }
	;

/* statement returning a value: assignments, function calls, etc. */
rstmt: id '=' exp		{		$$ = tstore(tname(tautodecl($1)), $3); }
	| id TOK_PLUSEQU exp {		$$ = topassign(tname(tlookup($1)), $3, OpPlus); }
	| id TOK_MINUSEQU exp {		$$ = topassign(tname(tlookup($1)), $3, OpMinus); }
	| id TOK_MULEQU exp {		$$ = topassign(tname(tlookup($1)), $3, OpMul); }
	| id TOK_DIVEQU exp {		$$ = topassign(tname(tlookup($1)), $3, OpDiv); }

	| id '(' expl ')' {
								sym = lookup($1, YES);
								if (sym == NULL) {
									$$ = tcall($3, $1);
								}
								else {
									MPRINT1("function call to '%s()'", $1);
									$$ = tfunc($3, sym->tree);
								}
							}

/* $2 will be the end of a linked list of tlistelem nodes */
/* XXX: This causes 1 reduce/reduce conflict on '}'  How bad is this?  -JGG */
	| '{' expl '}' 	{ 				$$ = tlist($2); }

	| id '[' exp ']' 	{			$$ = tsubscriptread(tname(tlookup($1)), $3); }
	| id '[' exp ']' '=' exp {		$$ = tsubscriptwrite(tname(tlookup($1)), $3, $6); }
	;

/* identifier list */
idl: id					{ MPRINT("<idl>"); idlist[idcount++] = $1; }
	| idl ',' id		{ MPRINT("<idl, id>"); idlist[idcount++] = $3; }
	;

/* identifier */
id:  TOK_IDENT			{ MPRINT("<id>"); $$ = strsave(yytext); }
	;

/* expression list */
expl:	exp				{ MPRINT("<expl>"); $$ = tlistelem(temptylistelem(), $1); }
	| expl ',' exp		{ MPRINT("<expl, exp>"); $$ = tlistelem($1, $3); }
/* XXX causes reduce/reduce conflicts; don't need because str -> exp below
	| str	 				{ $$ = tlistelem(temptylistelem(), $1); }
	| expl ',' str		{ $$ = tlistelem($1, $3); }
*/
	| /* nothing */	{ MPRINT("<expl> (NULL)"); $$ = temptylistelem(); }
	;

/* string */
str:	TOK_STRING		{
								char *s = yytext + 1;
								s[strlen(s) - 1] = '\0';
								$$ = tstring(strsave(s));
							}
	;

/* Boolean expression */
bexp:	exp %prec LOWPRIO	{ $$ = $1; }
	| TOK_NOT bexp %prec TOK_UNEQU { $$ = tnot($2); }
	| bexp TOK_AND bexp	{ $$ = tcand($1, $3); }
	| bexp TOK_OR  bexp	{ $$ = tcor($1, $3); }
	| bexp TOK_EQU bexp	{ $$ = trel(OpEqual, $1, $3); }
	| exp TOK_UNEQU exp	{ $$ = trel(OpNotEqual, $1, $3); }
	| exp '<' exp			{ $$ = trel(OpLess, $1, $3); }
	| exp '>' exp			{ $$ = trel(OpGreater, $1, $3); }
	| exp TOK_LESSEQU exp { $$ = trel(OpLessEqual, $1, $3); }
	| exp TOK_GTREQU exp	{ $$ = trel(OpGreaterEqual, $1, $3); }
	| TOK_TRUE				{ $$ = trel(OpEqual, tconstf(1.0), tconstf(1.0)); }
	| TOK_FALSE				{ $$ = trel(OpNotEqual, tconstf(1.0), tconstf(1.0)); }
	;

/* expression */
exp: rstmt				{ $$ = $1; }
	| exp TOK_POW exp	{ $$ = top(OpPow, $1, $3); }
	| exp '*' exp		{ $$ = top(OpMul, $1, $3); }
	| exp '/' exp		{ $$ = top(OpDiv, $1, $3); }
	| exp '+' exp		{ $$ = top(OpPlus, $1, $3); }
	| exp '-' exp		{ $$ = top(OpMinus, $1, $3); }
	| exp '%' exp		{ $$ = top(OpMod, $1, $3); }
	| '(' bexp ')'		{ $$ = $2; }
	| str				{ $$ = $1; }
	| TOK_NUM			{
								double f = atof(yytext);
								$$ = tconstf(f);
							}
	| TOK_TRUE			{ $$ = tconstf(1.0); }
	| TOK_FALSE			{ $$ = tconstf(0.0); }
	| '-' exp %prec CASTTOKEN {
								/* tconstf is a dummy; makes exct_operator work */
								$$ = top(OpNeg, $2, tconstf(0.0));
							}
	| id					{
								$$ = tname(tlookup($1));
							}
	;

/* function declaration rules */

/* a <ret> needs to be the last statement in every function definition */

ret: TOK_RETURN exp			{	MPRINT("<ret>");
								$$ = treturn($2);
							}
	| TOK_RETURN exp ';'	{	MPRINT("<ret;>");
								$$ = treturn($2);
							}
	;

fstml:	stml ret			{	MPRINT("<stml,ret>");
								$$ = tfuncseq($1, $2);
							}
	| ret					{	MPRINT("<ret>");
								$$ = tfuncseq(temptylistelem(), $1);
							}
	;

/* an <arg> is always a type followed by an <id>, like "float length".  They
 are declared in PARAM scope because they will not be visible outside of the
 function definition.
 */

arg: TOK_FLOAT_DECL id		{ MPRINT("<arg>");
							  idlist[idcount++] = $2;
							  Tree decl = declare(MincFloatType); idcount = 0;
							  $$ = tseq(decl, tname(tlookup($2)));		// create a sequence of tdecl/tname nodes *** COULD THESE AUTO-DECLARE?
							}
	| TOK_STRING_DECL id	{ MPRINT("<arg>");
							  idlist[idcount++] = $2;
							  Tree decl = declare(MincStringType); idcount = 0;
							  $$ = tseq(decl, tname(tlookup($2)));
							}
	| TOK_HANDLE_DECL id	{ MPRINT("<arg>");
							  idlist[idcount++] = $2;
							  Tree decl = declare(MincHandleType); idcount = 0;
							  $$ = tseq(decl, tname(tlookup($2)));
							}
	;

/* an <argl> is one <arg> or a series of <arg>'s separated by commas */

argl: arg             { MPRINT("<argl>"); $$ = targlistelem(temptylistelem(), $1); }
	| argl ',' arg    { MPRINT("<argl,arg>"); $$ = targlistelem($1, $3); }
	;

/* a <fargl> is a argument list for a function definition, like (float f, string s).
   We store this list in an NodeArgList tree because it gives us the information
   about the types of each of the arguments at the point where the function is called.
 */

fargl: '(' argl ')'			{ MPRINT("<fargl>"); $$ = targlist($2); }
	| '(' ')'              	{ MPRINT("<fargl> (NULL)"); $$ = targlist(temptylistelem()); }
	;

fname: TOK_FLOAT_DECL id function { MPRINT("<fname>");
									$$ = functionName = strsave($2); funReturnType = MincFloatType; }
	| TOK_STRING_DECL id function { MPRINT("<fname>");
									$$ = functionName = strsave($2); funReturnType = MincStringType; }
	| TOK_HANDLE_DECL id function { MPRINT("<fname>");
									$$ = functionName = strsave($2); funReturnType = MincHandleType; }
	;

/* function block level counter */
function:  /* nothing */ {	if (flevel > 0) {
								minc_die("nested function decls not allowed");
							}
							flevel++; MPRINT1("<function> - flevel => %d", flevel);
						 }
;

fdef: fname fargl '{' fstml '}'	{
											MPRINT("<fdef>");
											level--; MPRINT1("level => %d", level);
											--flevel; MPRINT1("flevel => %d", flevel);
											sym = declareFunction($1, funReturnType);
											functionName = NULL;			/* now that we are past the argument list */
											funReturnType = MincVoidType; 	/* reset */
											if (sym != NULL) {
												sym->tree = tfdef($2, $4, $1);
											}
											/* because we're just a decl, and the tree is stored
											   in the Symbol, we do not return a Tree to the parser.
											 */
											$$ = tnoop();
										}
	| error fname fargl '{' stml '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $1); flerror = 1; }
	| error fname fargl '{' '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $1); flerror = 1; }
	;

%%

// N.B. Because we have no need for <id>'s to exist in our tree other than for the purpose
// of declaring variables, we shortcut here and do not rely on the recursive parser.  We
// create our own sequence of declaration nodes.

static Tree declare(MincDataType type)
{
	MPRINT2("declare(type=%d, idcount=%d)", type, idcount);
	int i;
	
	Tree t = tnoop();	// end of the list
	
	for (i = 0; i < idcount; i++) {
		Tree decl = tdecl(idlist[i], type);
		t = tseq(t, decl);
	}
	return t;
}

static struct symbol *
declareFunction(const char *name, MincDataType returnType)
{
	MPRINT2("declareFunction('%s', type=%d)", name, returnType);
	assert(current_scope() == 0);	// until I allow nested functions
	Symbol *sym = lookup(name, FALSE);	// only look at current global level
	if (sym != NULL) {
		minc_die("function %s() is already declared", name);
		return NULL;
	}
	else {
		sym = install(name);		// all functions global for now
		sym->type = returnType;
	}
	return sym;
}

#define FREE_TREES_AT_END

static Tree
go(Tree t1)
{
	MPRINT1("--> go(%p)", t1);
	if (level == 0) {
		exct(t1);
#ifndef FREE_TREES_AT_END
		free_tree(t1);
#endif
	}
	MPRINT1("<-- go(%p)", t1);
	return t1;
}

int yywrap()
{
	return 1;
}

static void cleanup()
{
	MPRINT1("cleanup: yy_init = %d", yy_init);
	MPRINT1("Freeing program tree %p", program);
#ifdef FREE_TREES_AT_END
	print_tree(program);
    free_tree(program);
#else
	efree(program);
#endif
	/* Reset all static state */
	comments = 0;	// from lex.yy.c
	cpcomments = 0;
	program = NULL;
	idcount = 0;
	flerror = 0;
	level = 0;
#ifndef EMBEDDED
	/* BGG mm -- we need to keep the symbols for The Future */
	free_symbols();
	/* BGG mm -- I think this buffer gets reused, so we don't delete it */
	yy_delete_buffer(YY_CURRENT_BUFFER);
	YY_CURRENT_BUFFER_LVALUE = NULL;
#endif
}

#ifdef EMBEDDED

#warning DAS Make sure yylex_destroy() works
#define USE_YYLEX_DESTROY

// BGG mm -- for dynamic memory mgmt (double return for UG_INTRO() macro)
double minc_memflush()
{
	MPRINT("minc_memflush: Freeing parser memory");
#ifdef FREE_TREES_AT_END
	free_tree(program);
#else
	efree(program);
#endif
	program = NULL;
	free_symbols();
#ifdef USE_YYLEX_DESTROY
	yylex_destroy();
#else
	yy_delete_buffer(YY_CURRENT_BUFFER);
	YY_CURRENT_BUFFER_LVALUE = NULL;
	
	yy_init = 1;    /* whether we need to initialize */
	yy_start = 0;   /* start state number */
#endif
	return 1.0;
}
#endif
