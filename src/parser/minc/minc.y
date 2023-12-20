/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
%{
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "rename.h"
#include "bison_version.h"
#include "minc_internal.h"
#include "Node.h"
#include "Symbol.h"
#include "lex.yy.c"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef EMBEDDED
double minc_memflush();
#else
// in args.cpp
const char *lookup_token(const char *token, bool printWarning);
#endif
#ifdef __cplusplus
}
#endif
#define MDEBUG	/* turns on yacc debugging below */
#define DEBUG_ID    /* turns on printing of each ID found (assumes MDEBUG) */

#ifdef MDEBUG
// yydebug=1;
#define MPRINT(x) rtcmix_print("YACC: %s\n", x)
#define MPRINT1(x,y) rtcmix_print("YACC: " x "\n", y)
#define MPRINT2(x,y,z) rtcmix_print("YACC: " x "\n", y, z)
#else
#define MPRINT(x)
#define MPRINT1(x,y)
#define MPRINT2(x,y,z)
#endif

#ifdef DEBUG_ID
#define MPRINT_ID(id) MPRINT1("id\t\t\(%s)", yytext)
#else
#define MPRINT_ID(id) MPRINT("id")
#endif

#undef YYDEBUG
#define YYDEBUG 1
#define MAXTOK_IDENTLIST 200
#define TRUE 1
#define FALSE 0

static Node *	program;
static int		idcount = 0;
static char		*idlist[MAXTOK_IDENTLIST];  
static int		flerror;		/* set if there was an error during parsing */
static int		level = 0;		/* keeps track whether we are in a sub-block */
static int		flevel = 0;		/* > 0 if we are in a function decl block */
static const char *   sCurrentStructname = NULL;   /* set to struct name if we are in a struct decl block */
static int      xblock = 0;		/* 1 if we are entering a block preceeded by if(), else(), while(), or for() */
static bool     preserve_symbols = false;   /* what to do with symbol table at end of parse */
static void 	cleanup();
static void 	incrLevel();
static void		decrLevel();
static void     incrFunctionLevel();
static void     decrFunctionLevel();
static void     setStructName(const char *name);
static Node * declare(MincDataType type);
static Node * declareStructs(const char *typeName);
static Node * initializeStruct(const char *typeName, Node *initList);
static Node * parseArgumentQuery(const char *text, int *pOutErr);
static Node * parseScoreArgument(const char *text, int *pOutErr);
static Node * go(Node * t1);

%}

%left  <ival> LOWPRIO
%left  <ival> '='
%left  <ival> TOK_PLUSPLUS
%left  <ival> TOK_MINUSMINUS
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

%right <ival> TOK_STRUCT_DECL

%token <ival> TOK_FLOAT_DECL
%token <ival> TOK_STRING_DECL
%token <ival> TOK_HANDLE_DECL
%token <ival> TOK_LIST_DECL
%token <ival> TOK_MAP_DECL
%token <ival> TOK_MFUNC_DECL;
%token <ival> TOK_METHOD;
%token <ival> TOK_IDENT TOK_NUM TOK_ARG_QUERY TOK_ARG TOK_NOT TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_RETURN
%token <ival> TOK_TRUE TOK_FALSE TOK_STRING '{' '}'

%type  <node> stml stmt rstmt bexp expl exp expblk str ret bstml obj fexp fexpl subscript
%type  <node> fdecl sdecl hdecl ldecl mapdecl structdecl structinit mfuncdecl arg argl funcdef fstml fblock fargl funcname mbr mbrl structdef methodname methoddef
%type  <str> id structname

%destructor { MPRINT1("yydestruct unref'ing node %p\n", $$); RefCounted::unref($$); } stml stmt rstmt bexp expl exp str ret bstml fdecl sdecl hdecl ldecl mapdecl structdecl structinit mfuncdecl funcdef fstml arg argl fargl funcname mbr mbrl structdef obj fexp fexpl fblock expblk subscript methodname methoddef

%error-verbose

%%
/* program (the "start symbol") */
prg:	| stml				{ MPRINT("prg:"); program = $1; program->ref(); cleanup(); return 0; }
	;
 
/* statement list */
stml:	stmt				{ MPRINT("stml:	stmt"); $$ = $1; }
	| stmt ';'				{ MPRINT("stml:	stmt;"); $$ = $1; }
	| stml stmt				{ MPRINT("stml:	stml stmt"); $$ = new NodeSeq($1, $2); }
	| stml stmt ';'			{ MPRINT("stml:	stml stmt;"); $$ = new NodeSeq($1, $2); }
	;

/* statement */
stmt: rstmt					{ MPRINT("stmt: rstmt");	$$ = go($1); }
	| fdecl
	| sdecl
	| hdecl
	| ldecl
    | mapdecl
    | structdecl
    | structinit
    | mfuncdecl
	| TOK_IF level bexp stmt {	xblock = 1; MPRINT("IF bexp stmt");
								decrLevel();
								$$ = go(new NodeIf($3, $4));
								xblock = 0;
							}
	| TOK_IF level bexp stmt TOK_ELSE stmt { xblock = 1;
								decrLevel();
								$$ = go(new NodeIfElse($3, $4, $6));
								xblock = 0;
							}
	| TOK_WHILE level bexp stmt	{ xblock = 1;	MPRINT("WHILE bexp stmt");
								decrLevel();
								$$ = go(new NodeWhile($3, $4));
								xblock = 0;
							}
	| TOK_FOR level '(' stmt ';' bexp ';' stmt ')' stmt { xblock = 1;
								decrLevel();
								$$ = go(new NodeFor($4, $6, $8, $10));
								xblock = 0;
							}
	| bstml
    | funcdef               { MPRINT("stmt: funcdef"); $$ = go($1); }
	| ret
    | structdef
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
									$$ = go(new NodeBlock($3));
								}
	/* DAS handling empty block as special case to avoid need for empty stml's */
	| '{' '}'		{ 	MPRINT("bstml: {}");
								$$ = go(new NodeBlock(new NodeNoop()));
								}
	;

/* A return statement.  Only used inside functions. */

ret: TOK_RETURN exp			{	MPRINT("ret exp");
								MPRINT1("\tcalled at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									$$ = new NodeNoop();
								}
								else {
									$$ = new NodeRet($2);
								}
							}
	| TOK_RETURN exp ';'	{	MPRINT("ret exp;");
								MPRINT1("\tcalled at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									$$ = new NodeNoop();
								}
								else {
									$$ = new NodeRet($2);
								}
							}
	;

/* An identifier list is used to declare a variable of a particular type, e.g., "float gain" */
idl: id					{ MPRINT("idl: id"); idlist[idcount++] = $1; }
	| idl ',' id		{ MPRINT("idl: idl,id"); idlist[idcount++] = $3; }
	;

/* An identifier is any single text token */
id:  TOK_IDENT			{ MPRINT_ID(yytext); $$ = strsave(yytext); }
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
mapdecl:    TOK_MAP_DECL idl    {     MPRINT("mapdecl");
                                $$ = go(declare(MincMapType));
                                idcount = 0;
                              }
    ;

mfuncdecl:    TOK_MFUNC_DECL idl    {     MPRINT("mfuncdecl"); $$ = go(declare(MincFunctionType)); idcount = 0; }
    ;
    
/* statement nesting level counter.  This is an inline action between tokens. */
level:  /* nothing */ { incrLevel(); }
	;

subscript:  '[' exp ']'         {       MPRINT("subscript: [exp]"); $$ = $2; }

/* An obj is an id or anything that can be operator accessed via . or [] */
obj:    id                  {       MPRINT("obj: id");          $$ = new NodeLoadSym($1); }
    |   obj '.' id            {       MPRINT("obj: obj.id");      $$ = new NodeMemberAccess($1, $3);  }
    |   obj '[' exp ']'      {       MPRINT("obj: obj[exp]");    $$ = new NodeSubscriptRead($1, $3); }
    ;

/* An expression list block is used to initialize a list or a struct */
expblk: '{' level expl '}'    { MPRINT("expblk: {expl}");    decrLevel(); $$ = new NodeList($3); }
    ;

/* A function expression is an item that can appear in a set of function arguments */
fexp:   exp             { MPRINT("fexp: exp"); $$ = $1; }
    |   bexp            { MPRINT("fexp: bexp"); $$ = $1; }
    ;

/* A function expression list is a function expression or a series of comma-separated function expressions */
fexpl:  fexp            { MPRINT("fexpl: fexp"); $$ = new NodeListElem(new NodeEmptyListElem(), $1); }
    |   fexpl ',' fexp  {  MPRINT("fexpl: fexpl,fexp"); $$ = new NodeListElem($1, $3); }
    ;

/* An rstmt is statement returning a value, such as assignments, function calls, etc. */
rstmt: id '=' exp		{ MPRINT("rstmt: id = exp");		$$ = new NodeStore(new NodeAutoDeclLoadSym($1), $3); }
	| id TOK_PLUSEQU exp {		$$ = new NodeOpAssign(new NodeLoadSym($1), $3, OpPlus); }
	| id TOK_MINUSEQU exp {		$$ = new NodeOpAssign(new NodeLoadSym($1), $3, OpMinus); }
	| id TOK_MULEQU exp {		$$ = new NodeOpAssign(new NodeLoadSym($1), $3, OpMul); }
	| id TOK_DIVEQU exp {		$$ = new NodeOpAssign(new NodeLoadSym($1), $3, OpDiv); }

    /* Special-case rules for incrementing/decrementing an array access.  This is needed because the returned
       value from [exp] has no symbol associated with it.
    */
 	| TOK_PLUSPLUS obj subscript  { MPRINT("rstmt: TOK_PLUSPLUS obj subscript");
 	    $$ = new NodeSubscriptIncrement($2, $3, new NodeConstf(1.0));
 	}
 	| TOK_MINUSMINUS obj subscript  { MPRINT("rstmt: TOK_MINUSMINUS obj subscript");
 	    $$ = new NodeSubscriptIncrement($2, $3, new NodeConstf(-1.0));
 	}

    /* Generic rule for all other objs */
    | TOK_PLUSPLUS obj %prec CASTTOKEN { MPRINT("rstmt: TOK_PLUSPLUS obj");
        $$ = new NodeOpAssign($2, new NodeConstf(1.0), OpPlusPlus);
    }
    | TOK_MINUSMINUS obj %prec CASTTOKEN { MPRINT("rstmt: TOK_MINUSMINUS obj");
        $$ = new NodeOpAssign($2, new NodeConstf(1.0), OpMinusMinus);
    }

    /* A function can be an id, a member access on a struct/class, or a list element accessed by index, that can be followed by (args) or () */
    | obj '(' fexpl ')' {    MPRINT("rstmt: obj(fexpl)");
                                    $$ = new NodeCall($1, $3);
                                }
    | obj '(' ')'       { MPRINT("rstmt: obj()");
                                    $$ = new NodeCall($1, new NodeEmptyListElem());
                                }
        ;

    /* Special case: Assigning value to an array at an index */
	| obj '[' exp ']' '=' exp {
                                MPRINT("rstmt: obj[exp] = exp");
                                $$ = new NodeSubscriptWrite($1, $3, $6);
                            }
    /* Special case: Assigning value to an element in a struct.  Types can never be overwritten here. */
    | obj '.' id '=' exp       {
                                MPRINT("rstmt: obj.id = exp");
                                $$ = new NodeStore(new NodeMemberAccess($1, $3), $5, /* allowOverwrite = */ false);
                            }
	;

/* An expression list is an expression or set of expressions which will be wrapped in a block */
expl:	exp				{ MPRINT("expl: exp"); $$ = new NodeListElem(new NodeEmptyListElem(), $1); }
	| expl ',' exp		{ MPRINT("expl: expl,exp"); $$ = new NodeListElem($1, $3); }
	;

/* A string is quoted text */
str:	TOK_STRING		{
								char *s = yytext + 1;
								s[strlen(s) - 1] = '\0';
								$$ = new NodeString(strsave(s));
							}
	;

/* Boolean expression, before being wrapped in () */
bexp:	exp %prec LOWPRIO	{ MPRINT("bexp"); $$ = $1; }
	| TOK_NOT bexp %prec TOK_UNEQU { MPRINT("!bexp"); $$ = new NodeNot($2); }
	| bexp TOK_AND bexp	{ $$ = new NodeAnd($1, $3); }
	| bexp TOK_OR  bexp	{ $$ = new NodeOr($1, $3); }
	| bexp TOK_EQU bexp	{ $$ = new NodeRelation(OpEqual, $1, $3); }
	| exp TOK_UNEQU exp	{ $$ = new NodeRelation(OpNotEqual, $1, $3); }
	| exp '<' exp			{ $$ = new NodeRelation(OpLess, $1, $3); }
	| exp '>' exp			{ $$ = new NodeRelation(OpGreater, $1, $3); }
	| exp TOK_LESSEQU exp { $$ = new NodeRelation(OpLessEqual, $1, $3); }
	| exp TOK_GTREQU exp	{ $$ = new NodeRelation(OpGreaterEqual, $1, $3); }
	| TOK_TRUE				{ $$ = new NodeRelation(OpEqual, new NodeConstf(1.0), new NodeConstf(1.0)); }
	| TOK_FALSE				{ $$ = new NodeRelation(OpNotEqual, new NodeConstf(1.0), new NodeConstf(1.0)); }
	;

/* expression */
exp: rstmt				{ MPRINT("exp: rstmt"); $$ = $1; }
	| exp TOK_POW exp	{ $$ = new NodeOp(OpPow, $1, $3); }
	| exp '*' exp		{ $$ = new NodeOp(OpMul, $1, $3); }
	| exp '/' exp		{ $$ = new NodeOp(OpDiv, $1, $3); }
	| exp '+' exp		{ $$ = new NodeOp(OpPlus, $1, $3); }
	| exp '-' exp		{ $$ = new NodeOp(OpMinus, $1, $3); }
	| exp '%' exp		{ $$ = new NodeOp(OpMod, $1, $3); }
	| '(' bexp ')'		{ MPRINT("exp: (bexp)"); $$ = $2; } // bexp only an expression when wrapped
	| str				{ $$ = $1; }
	| TOK_NUM			{
							double f = atof(yytext);
							$$ = new NodeConstf(f);
						}
    /* DAS THESE ARE NOW PURE RIGHT-HAND-SIDE */
    | obj                   { MPRINT("exp: obj");   $$ = $1; }
    | expblk                { MPRINT("exp: expblk");   $$ = $1; }
	| '{' '}'				{ MPRINT("exp: {}");	$$ = new NodeList(new NodeEmptyListElem()); }
	| TOK_ARG_QUERY		    { $$ = parseArgumentQuery(yytext, &flerror); }
	| TOK_ARG			    { $$ = parseScoreArgument(yytext, &flerror); }
    
	| TOK_TRUE			{ $$ = new NodeConstf(1.0); }
	| TOK_FALSE			{ $$ = new NodeConstf(0.0); }
    | '-' exp %prec CASTTOKEN { MPRINT("exp: rstmt: '-' exp");
								/* NodeConstf is a dummy; makes exct_operator work */
								$$ = new NodeOp(OpNeg, $2, new NodeConstf(0.0));
							}
	;

/* Rules for declaring and defining structs */

/* An mbr is struct member declaration.
    It is either a type followed by an <id>, like "float length", or a method definition.
 */

mbr: TOK_FLOAT_DECL id  { MPRINT("mbr: decl");  $$ = new NodeMemberDecl($2, MincFloatType); }
    | TOK_STRING_DECL id    { MPRINT("mbr: decl");    $$ = new NodeMemberDecl($2, MincStringType); }
    | TOK_HANDLE_DECL id    { MPRINT("mbr: decl");    $$ = new NodeMemberDecl($2, MincHandleType); }
    | TOK_LIST_DECL id  { MPRINT("mbr: decl");  $$ = new NodeMemberDecl($2, MincListType); }
    | TOK_MAP_DECL id   { MPRINT("mbr: decl");   $$ = new NodeMemberDecl($2, MincMapType); }
    | TOK_MFUNC_DECL id { MPRINT("mbr: decl");   $$ = new NodeMemberDecl($2, MincFunctionType); }
    | structname id     { MPRINT("mbr: struct decl");   $$ = new NodeMemberDecl($2, MincStructType, $1); }     // member decl for struct includes struct type
    | TOK_METHOD methoddef  { MPRINT("mbr: methoddef"); $$ = $2; }    // $2 will be a NodeMethodDef instance
    ;

/* An mbrl is one <mbr> or a series of <mbr>'s separated by commas, for a struct definition,
    e.g. "float f, float g, string s"
 */

mbrl: mbr               { MPRINT("mbrl: mbr"); $$ = new NodeSeq(new NodeNoop(), $1); }
    | mbrl ',' mbr      { MPRINT("mbrl: mbrl,mbr"); $$ = new NodeSeq($1, $3); }
    ;

/* A structname is the rule for the beginning of a struct decl, e.g., "struct Foo" - we just return the id for the struct's name */

structname: TOK_STRUCT_DECL id %prec CASTTOKEN { MPRINT("structname"); $$ = $2; setStructName($2); }
    ;

/* A structdef is complete rule for a struct declaration, i.e., "struct Foo { <member decls> }" */

structdef: structname '{' mbrl '}'  { MPRINT("structdef");
                                        setStructName(NULL);
                                        $$ = go(new NodeStructDef($1, $3));
                                    }
    ;

/* A structdecl is a declaration of an object or list of objects to be type 'struct <SomeStructName>' */

structdecl: TOK_STRUCT_DECL id idl    { MPRINT("structdecl"); $$ = go(declareStructs($2)); idcount = 0; }
    ;

/* A structinit is a struct decl plus an initializer */
structinit: TOK_STRUCT_DECL id idl '=' expblk {   MPRINT("structinit: struct <type> id = expblk"); $$ = go(initializeStruct($2, $5)); idcount = 0; }

/* Rules for declaring and defining functions and methods */

/* function name, e.g. "list myfunction".  Used as first part of definition.
    TODO: This is where I would add the ability for a function to return an mfunction.
 */

funcname: TOK_FLOAT_DECL id { MPRINT("funcname"); incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($2), MincFloatType); }
    | TOK_STRING_DECL id { MPRINT("funcname"); incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($2), MincStringType); }
    | TOK_HANDLE_DECL id { MPRINT("funcname");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($2), MincHandleType); }
    | TOK_LIST_DECL id { MPRINT("funcname: returns list");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($2), MincListType); }
    | TOK_MAP_DECL id { MPRINT("funcname: returns map");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($2), MincMapType); }
    | TOK_STRUCT_DECL id id { MPRINT("funcname: returns struct");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl(strsave($3), MincStructType); }
    ;

/* method name, e.g. "list mymethod".  Used as first part of definition.
 */

methodname: TOK_FLOAT_DECL id { MPRINT("methodname"); incrFunctionLevel();
                            $$ = new NodeMethodDecl($2, sCurrentStructname, MincFloatType); }
    | TOK_STRING_DECL id { MPRINT("methodname"); incrFunctionLevel();
                            $$ = new NodeMethodDecl($2, sCurrentStructname, MincStringType); }
    | TOK_HANDLE_DECL id { MPRINT("methodname");  incrFunctionLevel();
                            $$ = new NodeMethodDecl($2, sCurrentStructname, MincHandleType); }
    | TOK_LIST_DECL id { MPRINT("methodname: returns list");  incrFunctionLevel();
                            $$ = new NodeMethodDecl($2, sCurrentStructname, MincListType); }
    | TOK_MAP_DECL id { MPRINT("methodname: returns map");  incrFunctionLevel();
                            $$ = new NodeMethodDecl($2, sCurrentStructname, MincMapType); }
    | TOK_STRUCT_DECL id id { MPRINT("methodname: returns struct");  incrFunctionLevel();
                            $$ = new NodeMethodDecl($3, sCurrentStructname, MincStructType); }
    ;

/* an <arg> is always a type followed by an <id>, like "float length".  They only occur in function and method definitions.
    The variables declared are not visible outside of the function.
 */

arg: TOK_FLOAT_DECL id      { MPRINT("arg");
                                    $$ = new NodeDecl($2, MincFloatType); }
    | TOK_STRING_DECL id    { MPRINT("arg");
                                    $$ = new NodeDecl($2, MincStringType); }
    | TOK_HANDLE_DECL id    { MPRINT("arg");
                                    $$ = new NodeDecl($2, MincHandleType); }
    | TOK_LIST_DECL id      { MPRINT("arg");
                                    $$ = new NodeDecl($2, MincListType); }
    | TOK_MAP_DECL id       { MPRINT("arg");
                                    $$ = new NodeDecl($2, MincMapType); }
    | TOK_STRUCT_DECL id id { MPRINT("arg: structname");
                                    $$ = new NodeStructDecl($3, $2); }
    | TOK_MFUNC_DECL id      { MPRINT("arg: mfunction");
                                    $$ = new NodeDecl($2, MincFunctionType); }
    ;

/* a <argl> is one <arg> or a series of <arg>'s separated by commas */

argl: arg     { MPRINT("argl: arg"); $$ = new NodeArgListElem(new NodeEmptyListElem(), $1); }
    | argl ',' arg    { MPRINT("argl: argl,arg"); $$ = new NodeArgListElem($1, $3); }
    ;

/* a <fargl> is a grouped argument list for a function definition, e.g. "(float f, string s)".
   We store this list in an NodeArgList tree because it gives us the information
   about the types of each of the arguments at the point where the function is called.
   When the function is executed, each NodeArgListElem executes, which declares the argument
   variable in the function's scope, then accesses it via lookup().
 */

fargl: '(' argl ')'		{ MPRINT("fargl: (argl)"); $$ = new NodeArgList($2); }
	| '(' ')'           { MPRINT("fargl: ()"); $$ = new NodeArgList(new NodeEmptyListElem()); }
	;

/* function body statement list.  Must be a statement list ending with a return statement */

fstml:	stml ret			{	MPRINT("fstml: stml ret");
									$$ = new NodeFuncBodySeq($1, $2);
							}
	| ret					{	MPRINT("fstml: ret");
									$$ = new NodeFuncBodySeq(new NodeEmptyListElem(), $1);
							}
	;

/* fblock is a function statement block including its curly braces */

fblock: '{' fstml '}' {     MPRINT("fblock"); $$ = $2; }

/* funcdef is a complete rule for a function definition, e.g. "list myfunction(string s) { ... }".
 */

funcdef: funcname fargl fblock	{ MPRINT("funcdef");
                                    decrFunctionLevel();
									$$ = new NodeFuncDef($1, $2, $3);
								}
/* These do not work (yet) and generate warnings, so commenting out for now
	| error funcname fargl '{' stml '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $2); flerror = 1; $$ = new NodeNoop(); }
	| error funcname fargl '{' '}'	{ minc_die("%s(): function body must end with 'return <exp>' statement", $2); flerror = 1; $$ = new NodeNoop(); }
*/
	;

/* methoddef is a complete rule for a struct method definition.  Looks the same as funcdef but only occurs within a struct definition.
 */

methoddef: methodname fargl fblock    { MPRINT("methoddef");
                                    decrFunctionLevel();
                                    $$ = new NodeMethodDef($1, $2, $3);
                                }
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

static void     incrFunctionLevel()
{
    if (flevel > 0) { minc_die("nested function decls not allowed"); }
    ++flevel; MPRINT1("flevel => %d", flevel);
    incrLevel();
}

static void        decrFunctionLevel()
{
    decrLevel();
    --flevel; MPRINT1("flevel => %d", flevel);
}

static void     setStructName(const char *name)
{
    if (name == NULL || sCurrentStructname == NULL) {   // never override one with another
        sCurrentStructname = name;
        if (name) {
            MPRINT1("storing struct name '%s'", name);
        }
        else {
            MPRINT("clearing stored struct name");
        }
    }
}

// N.B. Because we have no need for <id>'s to exist in our tree other than for the purpose
// of declaring variables, we shortcut here and do not rely on the recursive parser.  We
// create our own sequence of declaration nodes.

static Node * declare(MincDataType type)
{
	MPRINT2("declare(type=%d, idcount=%d)", type, idcount);
	assert(idcount > 0);
	
	Node * t = new NodeNoop();	// end of the list
	for (int i = 0; i < idcount; i++) {
		Node * decl = new NodeDecl(idlist[i], type);
		t = new NodeSeq(t, decl);
	}
	return t;
}

static Node * declareStructs(const char *typeName)
{
    MPRINT2("declareStructs(typeName='%s', idcount=%d)", typeName, idcount);
    assert(idcount > 0);
    Node * t = new NodeNoop();    // end of the list
    for (int i = 0; i < idcount; i++) {
        Node * decl = new NodeStructDecl(idlist[i], typeName);
        t = new NodeSeq(t, decl);
    }
    return t;
}

static Node * initializeStruct(const char *typeName, Node *initList)
{
    MPRINT2("initializeStruct(typeName='%s', initList=%p)", typeName, initList);
    assert(idcount > 0);
    if (idcount > 1) {
        minc_die("Only one struct variable can be initialized at a time");
    }
    Node * t = new NodeNoop();    // end of the list
    Node * decl = new NodeStructDecl(idlist[0], typeName, initList);
    t = new NodeSeq(t, decl);
    return t;
}

static Node * parseArgumentQuery(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    /* ?argument will return 1.0 if defined, else 0.0 */
    const char *token = strsave(text + 1);    // strip off '?'
    // returns NULL silently if not found
    const char *value = lookup_token(token, false);
    return new NodeConstf(value != NULL ? 1.0 : 0.0);
#else
    minc_warn("Argument variables not supported");
    *pOutErr = 1;
    return new NodeNoop();
#endif
}

static Node * parseListArgument(const char *text, int *pOutErr)
{
    Node *listNode = NULL;
    Node *listElem = new NodeEmptyListElem();
    ++text;         // get past open brace
    bool done = false;
    try {
        bool isNumber = false;      // until proven otherwise
        bool firstSign = true;     // watch for extra '-'
        int elemIndex = 0;          // start of text for each element
        for (int n = 0; !done; ++n) {
            switch (text[n]) {
            case '-':
                if (isNumber && !firstSign) {
                    minc_warn("Malformed list argument ignored");
                    throw 1;
                }
                firstSign = false;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
                isNumber = true;
                break;
            case '}':
                done = true;
                // fall through
            case ',':
                firstSign = true;      // clear this
                if (isNumber) {
                    double f = atof(&text[elemIndex]);
                    listElem = new NodeListElem(listElem, new NodeConstf(f));
                }
                else {
                    // Make copy so we can edit it.
                    char *vcopy = strdup(&text[elemIndex]);
                    char *term = strchr(vcopy, ',');
                    if (!term) term = strchr(vcopy, '}');
                    if (!term) {
                        minc_warn("Malformed list argument ignored");
                        free(vcopy);
                        throw 1;
                    }
                    *term = '\0';
                    if (strlen(vcopy) > 0) {
                        // strsave the list argument before adding to node
                        listElem = new NodeListElem(listElem, new NodeString(strsave(vcopy)));
                    }
                    free(vcopy);
                }
                // Advance the index
                elemIndex = n + 1;
                break;
            case '\0':
                minc_warn("Malformed list argument ignored");
                throw 1;
                break;
            default:
                isNumber = false;
                break;
            }
        }
        listNode = new NodeList(listElem);
    } catch (int &ii) {
        RefCounted::unref(listElem);
        *pOutErr = 1;
    }
    return listNode;
}

static Node * parseScoreArgument(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    const char *token = strsave(text + 1);    // strip off '$'
    const char *value = lookup_token(token, true);        // returns NULL with warning
    if (value != NULL) {
        // Value can be a list (e.g., "{1,2,3}"), a float, or a string
        
        // Check first for a list
        if (value[0] == '{') {
            Node *n = parseListArgument(value, pOutErr);
            return (n) ? n : new NodeNoop();
        }
        // Check to see if it can be coaxed into a number,
        int i, is_number = 1;
        for(i = 0; value[i] != '\0'; ++i) {
            if ('-' == value[i] && i == 0)
                continue;    // allow initial sign
            if (! (isdigit(value[i]) || '.' == value[i]) ) {
                is_number = 0;
                break;
            }
        }
        if (is_number) {
            double f = atof(value);
            return new NodeConstf(f);
        }
        // else we store this as a string constant.
        else {
            // Strip off extra "" if present
            if (value[0] == '"' && value[strlen(value)-1] == '"') {
                char *vcopy = strsave(value+1);
                *strrchr(vcopy, '"') = '\0';
                return new NodeString(vcopy);
            }
            else {
                return new NodeString(strsave(value));
            }
        }
    }
    else {
        *pOutErr = 1;
        return new NodeNoop();
    }
#else
    minc_warn("Argument variables not supported");
    *pOutErr = 1;
    return new NodeNoop();
#endif
}

static Node *
go(Node * t1)
{
	if (level == 0) {
		MPRINT1("--> go(%p)", t1);
		try {
			t1->exct();
		}
        catch(const RTException rtex) {
            MPRINT1("caught fatal exception: '%s' - cleaning up and re-throwing", rtex.mesg());
            t1->unref();
            t1 = NULL;
            cleanup();
            throw;
        }
		catch(...) {
			MPRINT("caught other exception - cleaning up and re-throwing");
			t1->unref();
			t1 = NULL;
			cleanup();
			throw;
		}
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
	rtcmix_debug("cleanup", "Freeing program tree %p", program);
    RefCounted::unref(program);
	program = NULL;
	/* Reset all static state */
	comments = 0;	// from lex.yy.c
	cpcomments = 0;
	xblock = 0;
	idcount = 0;
	flerror = 0;
	flevel = 0;
    sCurrentStructname = NULL;
	level = 0;
	include_stack_index = 0;
    if (!preserve_symbols) {
        free_symbols();
    }
#ifndef EMBEDDED
	/* BGG mm -- I think this buffer gets reused, so we don't delete it */
	yy_delete_buffer(YY_CURRENT_BUFFER);
	YY_CURRENT_BUFFER_LVALUE = NULL;
#endif
}

void preserveSymbols(bool preserve)
{
    preserve_symbols = preserve;
}

void reset_parser()
{
    rtcmix_debug("reset_parser", "resetting line number");
    flerror = 0;
    // Reset the line # every time a new score buffer is received
    yyset_lineno(1);
    // Special exported function from minc.l
    yy_clear_includes();
}

#ifdef EMBEDDED

#warning DAS Make sure yylex_destroy() works
#define USE_YYLEX_DESTROY

// BGG mm -- for dynamic memory mgmt (double return for UG_INTRO() macro)
double minc_memflush()
{
	rtcmix_debug("minc_memflush", "Freeing parser memory");
    RefCounted::unref(program);
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
