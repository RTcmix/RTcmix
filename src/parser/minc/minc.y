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

#undef MDEBUG	/* turns on yacc debugging below */

#ifdef MDEBUG
//int yydebug=1;
#define MPRINT(x) rtcmix_print("YACC: %s\n", x)
#define MPRINT1(x,y) rtcmix_print("YACC: " x "\n", y)
#define MPRINT2(x,y,z) rtcmix_print("YACC: " x "\n", y, z)
#else
#define MPRINT(x)
#define MPRINT1(x,y)
#define MPRINT2(x,y,z)
#endif

#define YYDEBUG 1
#define MAXTOK_IDENTLIST 200
#define TRUE 1
#define FALSE 0
#define CHECK_ERROR if (flerror) do { MPRINT("cleaning up after error"); cleanup(1); YYABORT; } while(0)

static Tree		program;
static int		idcount = 0;
static char		*idlist[MAXTOK_IDENTLIST];  
static int		flerror;		/* set if there was an error during parsing */
static int		level = 0;		/* keeps track whether we are in a sub-block */
static int		flevel = 0;		/* > 0 if we are in a function decl block */
static int      xblock = 0;		/* 1 if we are entering a block preceeded by if(), else(), while(), or for() */
static void 	cleanup();
static void 	incrLevel();
static void		decrLevel();
static Tree declare(MincDataType type);
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
%token <ival> TOK_LIST_DECL
%token <ival> TOK_IDENT TOK_NUM TOK_NOT TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_RETURN
%token <ival> TOK_TRUE TOK_FALSE TOK_STRING '{' '}'
%type  <trees> stml stmt rstmt bexp expl exp str ret bstml
%type  <trees> fdecl sdecl hdecl ldecl fdef fstml arg argl fargl fundecl
%type  <str> id
%error-verbose

%%
/* program (the "start symbol") */
prg:	| stml				{ MPRINT("prg:"); program = $1; cleanup(); return 0; }
	;
 
/* statement list */
stml:	stmt				{ MPRINT("stml:	stmt"); $$ = $1; }
	| stmt ';'				{ MPRINT("stml:	stmt;"); $$ = $1; }
	| stml stmt				{ MPRINT("stml:	stml stmt"); $$ = tseq($1, $2); }
	| stml stmt ';'			{ MPRINT("stml:	stml stmt;"); $$ = tseq($1, $2); }
	;

/* statement */
stmt: rstmt					{ MPRINT("rstmt");	$$ = go($1); }
	| fdecl
	| sdecl
	| hdecl
	| ldecl
	| TOK_IF level bexp stmt {	xblock = 1; MPRINT("IF bexp stmt");
								decrLevel();
								$$ = go(tif($3, $4));
								xblock = 0;
							}
	| TOK_IF level bexp stmt TOK_ELSE stmt { xblock = 1;
								decrLevel();
								$$ = go(tifelse($3, $4, $6));
								xblock = 0;
							}
	| TOK_WHILE level bexp stmt	{ xblock = 1;	MPRINT("WHILE bexp stmt");
								decrLevel();
								$$ = go(twhile($3, $4));
								xblock = 0;
							}
	| TOK_FOR level '(' stmt ';' bexp ';' stmt ')' stmt { xblock = 1;
								decrLevel();
								$$ = go(tfor($4, $6, $8, $10));
								xblock = 0;
							}
	| bstml
	| fdef
	| ret					{}
	| error TOK_FLOAT_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_STRING_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_HANDLE_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_LIST_DECL	{ flerror = 1; $$ = tnoop(); }
	| error TOK_IF		{ flerror = 1; $$ = tnoop(); }
	| error TOK_WHILE	{ flerror = 1; $$ = tnoop(); }
	| error TOK_FOR	{ flerror = 1; $$ = tnoop(); }
	| error '{'		{ flerror = 1; $$ = tnoop(); }
	| error TOK_ELSE	{ flerror = 1; $$ = tnoop(); }
	| error TOK_RETURN	{ flerror = 1; $$ = tnoop(); }
	| error ';'			{ flerror = 1; $$ = tnoop(); }
	;

/* block statement list
   This is tricky because we want to bump 'level' here (to avoid executing the contents of the list
   until the block is completely created) but we don't want to bump it twice for <if bexp bstml>.  So,
   if/else/while/for statements set xblock to 1 indicating the bump has already been done.  Only a 
   stand-alone block statement will do its own bump.
 */

bstml:	'{'			{ if (!xblock) incrLevel(); }
		stml
		'}'			{ 	MPRINT("bstml: { stml }"); MPRINT2("level = %d, xblock = %d", level, xblock);
									if (!xblock) { decrLevel(); }
									$$ = go(tblock($3));
								}
	;

/* A return statement.  Only used inside functions. */

ret: TOK_RETURN exp			{	MPRINT("ret exp");
								MPRINT1("called at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									$$ = tnoop();
								}
								else {
									$$ = treturn($2);
								}
							}
	| TOK_RETURN exp ';'	{	MPRINT("ret exp;");
								MPRINT1("called at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									$$ = tnoop();
								}
								else {
									$$ = treturn($2);
								}
							}
	;

/* variable declaration lists */

fdecl:	TOK_FLOAT_DECL idl	{ 	MPRINT("fdecl");
								$$ = go(declare(MincFloatType));
								idcount = 0;
							}	// e.g., "float x, y z"
	;
sdecl:	TOK_STRING_DECL idl	{ 	MPRINT("sdecl");
								$$ = go(declare(MincStringType));
								idcount = 0;
							}
	;
hdecl:	TOK_HANDLE_DECL idl	{ 	MPRINT("hdecl");
								$$ = go(declare(MincHandleType));
								idcount = 0;
							}
	;
ldecl:	TOK_LIST_DECL idl	{ 	MPRINT("ldecl");
								$$ = go(declare(MincListType));
								idcount = 0;
							}
;

/* statement nesting level counter */
level:  /* nothing */ { incrLevel(); }
	;

/* statement returning a value: assignments, function calls, etc. */
rstmt: id '=' exp		{ MPRINT("rstmt: id = exp");		$$ = tstore(tautoname($1), $3); }
	| id TOK_PLUSEQU exp {		$$ = topassign(tname($1), $3, OpPlus); }
	| id TOK_MINUSEQU exp {		$$ = topassign(tname($1), $3, OpMinus); }
	| id TOK_MULEQU exp {		$$ = topassign(tname($1), $3, OpMul); }
	| id TOK_DIVEQU exp {		$$ = topassign(tname($1), $3, OpDiv); }

	| id '(' expl ')' {			MPRINT("id(expl)");
								$$ = tcall($3, $1);
							}

/* $2 will be the end of a linked list of tlistelem nodes */
/* XXX: This causes 1 reduce/reduce conflict on '}'  How bad is this?  -JGG */
	| '{' level expl '}'	{ MPRINT("{expl}");	decrLevel(); $$ = tlist($3); }

	| id '[' exp ']' 	{			$$ = tsubscriptread(tname($1), $3); }
	| id '[' exp ']' '=' exp {		$$ = tsubscriptwrite(tname($1), $3, $6); }
	;

/* identifier list */
idl: id					{ MPRINT("idl: id"); idlist[idcount++] = $1; }
	| idl ',' id		{ MPRINT("idl: idl,id"); idlist[idcount++] = $3; }
	;

/* identifier */
id:  TOK_IDENT			{ MPRINT("id"); $$ = strsave(yytext); }
	;

/* expression list */
expl:	exp				{ MPRINT("expl: exp"); $$ = tlistelem(temptylistelem(), $1); }
	| expl ',' exp		{ MPRINT("expl: expl,exp"); $$ = tlistelem($1, $3); }
	| /* nothing */	{ MPRINT("expl: NULL"); $$ = temptylistelem(); }
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
exp: rstmt				{ MPRINT("exp: rstmt"); $$ = $1; }
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
								$$ = tname($1);
							}
	;

/* function declaration */

fundecl: TOK_FLOAT_DECL id function { MPRINT("fundecl");
									$$ = go(tfdecl(strsave($2), MincFloatType)); }
	| TOK_STRING_DECL id function { MPRINT("fundecl");
									$$ = go(tfdecl(strsave($2), MincStringType)); }
	| TOK_HANDLE_DECL id function { MPRINT("fundecl");
									$$ = go(tfdecl(strsave($2), MincHandleType)); }
	| TOK_LIST_DECL id function { MPRINT("fundecl");
									$$ = go(tfdecl(strsave($2), MincListType)); }
	;


/* an <arg> is always a type followed by an <id>, like "float length".  They only occur in function definitions, and
   the variables declared are not visible outside of the function definition.
 */

arg: TOK_FLOAT_DECL id		{ MPRINT("arg");
							  $$ = tdecl($2, MincFloatType);
							}
	| TOK_STRING_DECL id	{ MPRINT("arg");
							  $$ = tdecl($2, MincStringType);
							}
	| TOK_HANDLE_DECL id	{ MPRINT("arg");
							  $$ = tdecl($2, MincHandleType);
							}
	| TOK_LIST_DECL id		{ MPRINT("arg");
								$$ = tdecl($2, MincListType);
							}
	;

/* an <argl> is one <arg> or a series of <arg>'s separated by commas */

argl: arg             { MPRINT("argl: arg"); $$ = targlistelem(temptylistelem(), $1); }
	| argl ',' arg    { MPRINT("argl: argl,arg"); $$ = targlistelem($1, $3); }
	;

/* a <fargl> is a argument list for a function definition, like (float f, string s).
   We store this list in an NodeArgList tree because it gives us the information
   about the types of each of the arguments at the point where the function is called.
   When the function is executed, each NodeArgListElem executes, which declares the argument
   variable in the function's scope, then accesses it via lookup().
 */

fargl: '(' argl ')'			{ MPRINT("fargl: (argl)"); $$ = targlist($2); }
	| '(' ')'              	{ MPRINT("fargl: (NULL)"); $$ = targlist(temptylistelem()); }
	;

/* function block level counter */

function:  level {	MPRINT("function"); if (flevel > 0) {
								minc_die("nested function decls not allowed");
							}
							flevel++; MPRINT1("flevel => %d", flevel);
						 }
;

/* function statement list must be a statement list ending with a return statement */

fstml:	stml ret			{	MPRINT("fstml: stml,ret");
									$$ = tfuncseq($1, $2);
							}
	| ret					{	MPRINT("fstml: ret");
									$$ = tfuncseq(temptylistelem(), $1);
							}
	;

/* the full rule for a function declaration/definition */

fdef: fundecl fargl '{' fstml '}'	{
									MPRINT("fdef");
									decrLevel();
									--flevel; MPRINT1("flevel => %d", flevel);
									go(tfdef($1, $2, $4));
									/* because we're just a decl, and the tree is stored
									   in the Symbol, we do not return a Tree to the parser.
									 */
									$$ = tnoop();
								}
	| error fundecl fargl '{' stml '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $2); flerror = 1; }
	| error fundecl fargl '{' '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $2); flerror = 1; }
	;

%%

static void 	incrLevel()
{
	++level; MPRINT1("level => %d", level);
}

static void		decrLevel()
{
	--level; MPRINT1("level => %d", level);
}

// N.B. Because we have no need for <id>'s to exist in our tree other than for the purpose
// of declaring variables, we shortcut here and do not rely on the recursive parser.  We
// create our own sequence of declaration nodes.

static Tree declare(MincDataType type)
{
	MPRINT2("declare(type=%d, idcount=%d)", type, idcount);
	int i;
	
	assert(idcount > 0);
	
	Tree t = tnoop();	// end of the list
	for (i = 0; i < idcount; i++) {
		Tree decl = tdecl(idlist[i], type);
		t = tseq(t, decl);
	}
	return t;
}

#define FREE_TREES_AT_END

static Tree
go(Tree t1)
{
	if (level == 0) {
		MPRINT1("--> go(%p)", t1);
		exct(t1);
#ifndef FREE_TREES_AT_END
		free_tree(t1);
#endif
		MPRINT1("<-- go(%p)", t1);
	}
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

// BGGx ww VS doesn't like #warning
//#warning DAS Make sure yylex_destroy() works
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

void reset_parser()
{
	set_rtcmix_error(0);
	flerror = 0;
	// Reset the line # every time a new score buffer is received
	yyset_lineno(1);
}
