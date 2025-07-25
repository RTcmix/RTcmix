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
double minc_memflush(double p[], int n_args);
#else
// in args.cpp
const char *lookup_token(const char *token, bool printWarning);
#endif
#ifdef __cplusplus
}
#endif
#undef MDEBUG	/* turns on yacc debugging below */
#undef DEBUG_ID    /* turns on printing of each ID found (assumes MDEBUG) */

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
%right '=' TOK_PLUSEQU TOK_MINUSEQU TOK_MULEQU TOK_DIVEQU   // assignment-like, should be right-associative
%right TOK_PLUSPLUS TOK_MINUSMINUS      // prefix unary ++/--
%left  TOK_OR
%left  TOK_AND
%right '?' ':'
%nonassoc TOK_EQU TOK_UNEQU
%nonassoc '<' '>' TOK_LESSEQU TOK_GTREQU
%left  '+' '-'
%left  '*' '/' '%'

%right TOK_POW
%right  <ival> CASTTOKEN TOK_NOT
%right <ival> TOK_STRUCT_DECL
%right <ival> TOK_BASE_DECL

%token <ival> TOK_FLOAT_DECL
%token <ival> TOK_STRING_DECL
%token <ival> TOK_HANDLE_DECL
%token <ival> TOK_LIST_DECL
%token <ival> TOK_MAP_DECL
%token <ival> TOK_MFUNC_DECL;
%token <ival> TOK_METHOD;
%token <ival> TOK_IDENT TOK_NUM TOK_ARG_QUERY TOK_ARG TOK_IF TOK_ELSE TOK_FOR TOK_WHILE TOK_RETURN
%token <ival> TOK_TRUE TOK_FALSE TOK_STRING '{' '}'

%type  <node> stml stmt rstmt expl exp expblk str ret bstml obj fexp fexpl func fcall mcall subscript ternary
%type  <node> decl fdecl sdecl hdecl ldecl mapdecl structdecl structinit mfuncdecl arg argl funcdef fblock fargl funcname mbr mbrl structdef methodname methoddef
%type  <str> id structname basename

%destructor { MPRINT1("yydestruct unref'ing node %p\n", $$); RefCounted::unref($$); } stml stmt rstmt expl exp str ret bstml fdecl sdecl hdecl ldecl ternary mapdecl structdecl structinit mfuncdecl funcdef arg argl fargl funcname mbr mbrl structdef obj fexp fexpl fblock expblk subscript methodname methoddef

%error-verbose

%%
/* program (the "start symbol") */
prg:	| stml				{ MPRINT("prg:"); program = $1; program->ref(); cleanup(); return 0; }
	;
 
/* statement list */
stml:	stmt				{ MPRINT("stmt -> stml"); $$ = $1; }
	| stmt ';'				{ MPRINT("stmt; -> stml"); $$ = $1; }
	| stml stmt				{ MPRINT("stml stmt -> stml"); $$ = new NodeSeq($1, $2); }
	| stml stmt ';'			{ MPRINT("stml stmt; -> stml"); $$ = new NodeSeq($1, $2); }
	;

/* statement */
stmt: decl                 { MPRINT("decl -> stmt"); $$ = $1; }
    | structdecl           { MPRINT("structdecl -> stmt"); $$ = $1; } /* separated out because we allow "struct Foo f = {...}" */
    | decl '=' exp         {
                              minc_die("Declarations may not include initializers.");
                              flerror = 1;
                              $$ = new NodeNoop();
                          }
    | structinit
	| bstml                 { MPRINT("bstml -> stmt"); $$ = $1; }
	| TOK_IF level exp stmt {	xblock = 1; MPRINT("IF exp stmt -> smtm");
								decrLevel();
								$$ = go(new NodeIf($3, $4));
								xblock = 0;
							}
	| TOK_IF level exp stmt TOK_ELSE stmt { xblock = 1;
								decrLevel();
								$$ = go(new NodeIfElse($3, $4, $6));
								xblock = 0;
							}
	| TOK_WHILE level exp stmt	{ xblock = 1;	MPRINT("WHILE exp stmt -> stmt");
								decrLevel();
								$$ = go(new NodeWhile($3, $4));
								xblock = 0;
							}
	| TOK_FOR level '(' stmt ';' exp ';' stmt ')' stmt { xblock = 1;
								decrLevel();
								$$ = go(new NodeFor($4, $6, $8, $10));
								xblock = 0;
							}
	| ret                   { MPRINT("ret -> stmt");
	                          if (level == 0) {
	                            minc_die("return statements are not allowed in main score"); $$ = new NodeNoop();
	                          }
	                          else { $$ = $1; }
	                        }
    | rstmt					{ MPRINT("rstmt -> stmt");	$$ = go($1); }
    | funcdef               { MPRINT("funcdef -> stmt"); $$ = go($1); }
    | structdef             { MPRINT("structdef -> stmt"); $$ = $1; }
	;

/* block statement list
   This is tricky because we want to bump 'level' here (to avoid executing the contents of the list
   until the block is completely created) but we don't want to bump it twice for <if exp bstml>.  So,
   if/else/while/for statements set xblock to 1 indicating the bump has already been done.  Only a 
   stand-alone block statement will do its own bump.
 */

bstml:	'{'			{ if (!xblock) incrLevel(); }
		stml
		'}'			{ 	MPRINT("{ stml } -> bstml"); MPRINT2("level = %d, xblock = %d", level, xblock);
									if (!xblock) { decrLevel(); }
									$$ = go(new NodeBlock($3));
								}
	/* DAS handling empty block as special case to avoid need for empty stml's */
	| '{' '}'		{ 	MPRINT("{} -> bstml");
								$$ = go(new NodeBlock(new NodeNoop()));
								}
	;

/* A return statement.  Only used inside functions. */

ret: TOK_RETURN exp			{	MPRINT("return exp -> ret");
								MPRINT1("\tcalled at level %d", level);
								$$ = new NodeRet($2);
							}
	| TOK_RETURN exp ';'	{	MPRINT("return exp; -> ret");
								MPRINT1("\tcalled at level %d", level);
								$$ = new NodeRet($2);
							}
	| TOK_RETURN            {   MPRINT("return");
                                if (level > 0) { minc_die("return statements must return a value"); } $$ = new NodeNoop();
                            }
	| TOK_RETURN ';'        {   MPRINT("return;");
                                if (level > 0) { minc_die("return statements must return a value"); } $$ = new NodeNoop();
                            }
	;

/* An identifier list is used to declare a variable of a particular type, e.g., "float gain" */
idl: id					{ MPRINT("id -> idl"); idlist[idcount++] = $1; }
	| id ',' idl		{ MPRINT("id,idl -> idl"); idlist[idcount++] = $1; }
	;

/* An identifier is any single text token */
id:  TOK_IDENT			{ MPRINT_ID(yytext); $$ = strsave(yytext); }
	;

/* variable declaration lists (except struct, which behaves differently) */
decl:   fdecl
    |   sdecl
    |   hdecl
    |   ldecl
    |   mapdecl
    |   mfuncdecl
    ;

fdecl:	TOK_FLOAT_DECL idl	{ 	MPRINT("fdecl -> decl");
								$$ = go(declare(MincFloatType));
								idcount = 0;
							}	// e.g., "float x, y z"
	;
sdecl:	TOK_STRING_DECL idl	{ 	MPRINT("sdecl -> decl");
								$$ = go(declare(MincStringType));
								idcount = 0;
							}
	;
hdecl:	TOK_HANDLE_DECL idl	{ 	MPRINT("hdecl -> decl");
								$$ = go(declare(MincHandleType));
								idcount = 0;
							}
	;
ldecl:	TOK_LIST_DECL idl	{ 	MPRINT("ldecl -> decl");
								$$ = go(declare(MincListType));
								idcount = 0;
							}
    ;
mapdecl:    TOK_MAP_DECL idl    {     MPRINT("mapdecl -> decl");
                                $$ = go(declare(MincMapType));
                                idcount = 0;
                              }
    ;

mfuncdecl:    TOK_MFUNC_DECL idl    {     MPRINT("mfuncdecl -> decl"); $$ = go(declare(MincFunctionType)); idcount = 0; }
    ;
    
/* statement nesting level counter.  This is an inline action between tokens. */
level:  /* nothing */ { incrLevel(); }
	;

subscript:  '[' exp ']'         {       MPRINT("[exp] -> subscript"); $$ = $2; }

/* An obj is an id or anything that can be operator accessed via . or [] */
obj:    id                  {       MPRINT("id -> obj");          $$ = new NodeLoadSym($1); }
    |   obj '.' id            {       MPRINT("obj.id -> obj");      $$ = new NodeMemberAccess($1, $3);  }
    |   obj '[' exp ']'      {       MPRINT("obj[exp] -> obj");    $$ = new NodeSubscriptRead($1, $3); }
    ;

/* An expression list block is used to initialize a list or a struct */
expblk: '{' level expl '}'    { MPRINT("{expl} -> expblk");    decrLevel(); $$ = new NodeList($3); }
    ;

/* A function expression is an item that can appear in a set of function arguments */
fexp:   exp             { MPRINT("exp -> fexp"); $$ = $1; }
    ;

/* A function expression list is a function expression or a series of comma-separated function expressions */
fexpl:  fexp            { MPRINT("fexp -> fexpl"); $$ = new NodeListElem(new NodeEmptyListElem(), $1); }
    |   fexpl ',' fexp  {  MPRINT("fexpl,fexp -> fexpl"); $$ = new NodeListElem($1, $3); }
    ;

/* A function is a set of function arguments inside parentheses */
func:   '(' fexpl ')' {    MPRINT("(fexpl) -> func"); $$ = $2; }
    |   '(' ')'       {     MPRINT("() -> func"); $$ = new NodeEmptyListElem();  }
    ;

/* A function call is an id followed by (args) or () */
fcall:  id func       {    MPRINT("id func -> fcall"); $$ = new NodeFunctionCall(new NodeLoadSym($1), $2); }
    |   fcall func    {    MPRINT("fcall func -> fcall"); $$ = new NodeFunctionCall($1, $2); }    /* calling a function on the returned value of a function */
    |   obj subscript func { MPRINT("obj subscript func -> fcall"); $$ = new NodeFunctionCall(new NodeSubscriptRead($1, $2), $3); }
    ;

/* A method is a function call on an object using the dot operator. The object can be an id, a member access on a
   struct/class, or a list element accessed by index.  The id is the string representing the method */
mcall: obj '.' id func {  MPRINT("obj.id func -> mcall"); $$ = new NodeMethodCall($1, $3, $4); }
    ;

/* New ternary statement - Doug likes having these */
ternary: exp '?' exp ':' exp { $$ = new NodeTernary($1, $3, $5); }
    ;

/* An rstmt is statement returning a value, such as assignments, function calls, etc. */
rstmt:
    id '=' exp		{ MPRINT("id = exp -> rstmt");		$$ = new NodeStore(new NodeAutoDeclLoadSym($1), $3); }
    /* Special case: Assigning value to an element in a struct.  Types can never be overwritten here. */
    | obj '.' id '=' exp       {
                                MPRINT("obj.id = exp -> rstmt");
                                $$ = new NodeStore(new NodeMemberAccess($1, $3), $5, /* allowOverwrite = */ false);
                            }
    /* Special case: Assigning value to an array at an index */
	| obj '[' exp ']' '=' exp {
                                MPRINT("obj[exp] = exp -> rstmt");
                                $$ = new NodeSubscriptWrite($1, $3, $6);
                            }
	| obj TOK_PLUSEQU exp {	    $$ = new NodeOpAssign($1, $3, OpPlus); }
	| obj TOK_MINUSEQU exp {	$$ = new NodeOpAssign($1, $3, OpMinus); }
	| obj TOK_MULEQU exp {		$$ = new NodeOpAssign($1, $3, OpMul); }
	| obj TOK_DIVEQU exp {		$$ = new NodeOpAssign($1, $3, OpDiv); }
    /* Special-case rules for operating on an array access.  This is needed because the returned
       value from array[exp] has no symbol associated with it.
    */
	| obj subscript TOK_PLUSEQU exp { MPRINT("obj subscript tok_PLUSEQU EXP -> rstmt");
     	$$ = new NodeSubscriptOpAssign($1, $2, $4, OpPlus);
	}
	| obj subscript TOK_MINUSEQU exp { MPRINT("obj subscript tok_MINUSEQU EXP -> rstmt");
     	$$ = new NodeSubscriptOpAssign($1, $2, $4, OpMinus);
	}
	| obj subscript TOK_MULEQU exp { MPRINT("obj subscript tok_MULEQU EXP -> rstmt");
     	$$ = new NodeSubscriptOpAssign($1, $2, $4, OpMul);
	}
	| obj subscript TOK_DIVEQU exp { MPRINT("obj subscript tok_DIVEQU EXP -> rstmt");
     	$$ = new NodeSubscriptOpAssign($1, $2, $4, OpDiv);
	}
    /* Generic rule for incrementing/decrementing */
    | TOK_PLUSPLUS obj %prec CASTTOKEN { MPRINT("tok_PLUSPLUS OBJ -> rstmt");
        $$ = new NodeOpAssign($2, new NodeConstf(1.0), OpPlusPlus);
    }
    | TOK_MINUSMINUS obj %prec CASTTOKEN { MPRINT("tok_MINUSMINUS OBJ -> rstmt");
        $$ = new NodeOpAssign($2, new NodeConstf(1.0), OpMinusMinus);
    }
    /* Special-case rules for incrementing/decrementing an array access.  This is needed because the returned
       value from array[exp] has no symbol associated with it.
    */
 	| TOK_PLUSPLUS obj subscript  { MPRINT("tok_PLUSPLUS OBJ subscript -> rstmt");
 	    $$ = new NodeSubscriptOpAssign($2, $3, new NodeConstf(1.0), OpPlus);
 	}
 	| TOK_MINUSMINUS obj subscript  { MPRINT("tok_MINUSMINUS OBJ subscript -> rstmt");
 	    $$ = new NodeSubscriptOpAssign($2, $3, new NodeConstf(1.0), OpMinus);
 	}

    |   fcall        {  MPRINT("fcall -> rstmt"); $$ = $1; }
    |   mcall        {  MPRINT("mcall -> rstmt"); $$ = $1; }
	;

/* An expression list is an expression or set of expressions which will be wrapped in a block */
expl:	exp				{ MPRINT("exp -> expl"); $$ = new NodeListElem(new NodeEmptyListElem(), $1); }
	| expl ',' exp		{ MPRINT("expl,exp -> expl"); $$ = new NodeListElem($1, $3); }
	;

/* A string is quoted text */
str:	TOK_STRING		{   char *s = yytext + 1;
                            s[strlen(s) - 1] = '\0';
                            $$ = new NodeString(strsave(s));
						}
	;

/* expression, now including all boolean expressions as well */
exp:
    rstmt                             { MPRINT("rstmt -> exp"); $$ = $1; }
  | ternary                           { MPRINT("ternary -> exp"); $$ = $1; }

  // Arithmetic
  | exp TOK_POW exp                   { $$ = new NodeOp(OpPow, $1, $3); }
  | exp '*' exp                       { $$ = new NodeOp(OpMul, $1, $3); }
  | exp '/' exp                       { $$ = new NodeOp(OpDiv, $1, $3); }
  | exp '%' exp                       { $$ = new NodeOp(OpMod, $1, $3); }
  | exp '+' exp                       { $$ = new NodeOp(OpPlus, $1, $3); }
  | exp '-' exp                       { $$ = new NodeOp(OpMinus, $1, $3); }

  // Logical and relational (formerly in bexp)
  | TOK_NOT exp %prec TOK_UNEQU       { MPRINT("!exp -> exp"); $$ = new NodeNot($2); }
  | exp TOK_AND exp                   { $$ = new NodeAnd($1, $3); }
  | exp TOK_OR exp                    { $$ = new NodeOr($1, $3); }
  | exp TOK_EQU exp                   { $$ = new NodeRelation(OpEqual, $1, $3); }
  | exp TOK_UNEQU exp                 { $$ = new NodeRelation(OpNotEqual, $1, $3); }
  | exp '<' exp                       { $$ = new NodeRelation(OpLess, $1, $3); }
  | exp '>' exp                       { $$ = new NodeRelation(OpGreater, $1, $3); }
  | exp TOK_LESSEQU exp               { $$ = new NodeRelation(OpLessEqual, $1, $3); }
  | exp TOK_GTREQU exp                { $$ = new NodeRelation(OpGreaterEqual, $1, $3); }

  // Constants and literals
  | TOK_TRUE                          { $$ = new NodeConstf(1.0); }
  | TOK_FALSE                         { $$ = new NodeConstf(0.0); }

  // Grouping
  | '(' exp ')'                       { MPRINT("(exp) -> exp"); $$ = $2; }

  // Unary minus (uses dummy NodeConstf to satisfy operator format)
  | '-' exp %prec CASTTOKEN           {
                                        MPRINT("'-' exp -> exp");
                                        $$ = new NodeOp(OpNeg, $2, new NodeConstf(0.0));
                                      }

  // Data and special types
  | str                               { $$ = $1; }
  | TOK_NUM                           {
                                        double f = atof(yytext);
                                        $$ = new NodeConstf(f);
                                      }
  | obj                               { MPRINT("obj -> exp"); $$ = $1; }
  | expblk                            { MPRINT("expblk -> exp"); $$ = $1; }
  | '{' '}'                           { MPRINT("{} -> exp"); $$ = new NodeList(new NodeEmptyListElem()); }
  | TOK_ARG_QUERY                     { $$ = parseArgumentQuery(yytext, &flerror); }
  | TOK_ARG                           { $$ = parseScoreArgument(yytext, &flerror); }
;

/* Rules for declaring and defining structs */

/* An mbr is struct member declaration.
    It is either a type followed by an <id>, like "float length", or a method definition.
 */

mbr: TOK_FLOAT_DECL id  { MPRINT("decl -> mbr");  $$ = new NodeMemberDecl($2, MincFloatType); }
    | TOK_STRING_DECL id    { MPRINT("decl -> mbr");    $$ = new NodeMemberDecl($2, MincStringType); }
    | TOK_HANDLE_DECL id    { MPRINT("decl -> mbr");    $$ = new NodeMemberDecl($2, MincHandleType); }
    | TOK_LIST_DECL id  { MPRINT("decl -> mbr");  $$ = new NodeMemberDecl($2, MincListType); }
    | TOK_MAP_DECL id   { MPRINT("decl -> mbr");   $$ = new NodeMemberDecl($2, MincMapType); }
    | TOK_MFUNC_DECL id { MPRINT("decl -> mbr");   $$ = new NodeMemberDecl($2, MincFunctionType); }
    | structname id     { MPRINT("struct decl -> mbr");   $$ = new NodeMemberDecl($2, MincStructType, $1); }     // member decl for struct includes struct type
    | TOK_METHOD methoddef  { MPRINT("methoddef -> mbr"); $$ = $2; }    // $2 will be a NodeMethodDef instance
    ;

/* An mbrl is one <mbr> or a series of <mbr>'s separated by commas, for a struct definition,
    e.g. "float f, float g, string s"
 */

mbrl: mbr               { MPRINT("mbr -> mbrl"); $$ = new NodeSeq(new NodeNoop(), $1); }
    | mbrl ',' mbr      { MPRINT("mbrl,mbr -> mbrl"); $$ = new NodeSeq($1, $3); }
    ;

/* A structname is the rule for the beginning of a struct decl, e.g., "struct Foo" - we just return the id for the struct's name */

structname: TOK_STRUCT_DECL id %prec CASTTOKEN { MPRINT("structname"); $$ = $2; setStructName($2); }
    ;

/* A basename is the rule for the base struct type when declaring a new struct type */

basename: TOK_BASE_DECL id %prec CASTTOKEN { MPRINT("basename"); $$ = $2;  }
    ;

/* A structdef is complete rule for a struct declaration, i.e., "struct Foo { <member decls> }" */

structdef: structname '{' mbrl '}'  { MPRINT("structdef");
                                        setStructName(NULL);
                                        $$ = go(new NodeStructDef($1, $3));
                                    }
    |      structname basename '{' mbrl '}'  { MPRINT("structdef (with base class)");
                                               setStructName(NULL);
                                               $$ = go(new NodeStructDef($1, $4, $2));
                                             }
    ;

/* A structdecl is a declaration of an object or list of objects to be type 'struct <SomeStructName>' */

structdecl: TOK_STRUCT_DECL id idl    { $$ = go(declareStructs($2)); idcount = 0; }
    ;

/* A structinit is a struct decl plus an initializer */
structinit: TOK_STRUCT_DECL id idl '=' expblk {   MPRINT("structinit: struct <type> id = expblk -> stmt"); $$ = go(initializeStruct($2, $5)); idcount = 0; }

/* Rules for declaring and defining functions and methods */

/* function name, e.g. "list myfunction".  Used as first part of definition. */

funcname: TOK_FLOAT_DECL id { MPRINT("funcname"); incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincFloatType); }
    | TOK_STRING_DECL id { MPRINT("funcname"); incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincStringType); }
    | TOK_HANDLE_DECL id { MPRINT("funcname");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincHandleType); }
    | TOK_LIST_DECL id { MPRINT("funcname: returns list");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincListType); }
    | TOK_MFUNC_DECL id { MPRINT("funcname: returns mfunction"); incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincFunctionType); }
    | TOK_MAP_DECL id { MPRINT("funcname: returns map");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl($2, MincMapType); }
    | TOK_STRUCT_DECL id id { MPRINT("funcname: returns struct");  incrFunctionLevel();
                                    $$ = new NodeFuncDecl($3, MincStructType); }
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
    | TOK_STRUCT_DECL id id { MPRINT("structname -> arg");
                                    $$ = new NodeStructDecl($3, $2); }
    | TOK_MFUNC_DECL id      { MPRINT("mfunction -> arg");
                                    $$ = new NodeDecl($2, MincFunctionType); }
    ;

/* a <argl> is one <arg> or a series of <arg>'s separated by commas */

argl: arg     { MPRINT("arg -> argl"); $$ = new NodeArgListElem(new NodeEmptyListElem(), $1); }
    | argl ',' arg    { MPRINT("argl,arg -> argl"); $$ = new NodeArgListElem($1, $3); }
    ;

/* a <fargl> is a grouped argument list for a function definition, e.g. "(float f, string s)".
   We store this list in an NodeArgList tree because it gives us the information
   about the types of each of the arguments at the point where the function is called.
   When the function is executed, each NodeArgListElem executes, which declares the argument
   variable in the function's scope, then accesses it via lookup().
 */

fargl: '(' argl ')'		{ MPRINT("(argl) -> fargl"); $$ = new NodeArgList($2); }
	| '(' ')'           { MPRINT("() -> fargl"); $$ = new NodeArgList(new NodeEmptyListElem()); }
	;

/* fblock is a function body statement list in a block including its curly braces.
   The statement list must end with a return statement. */

fblock: '{' stml ret '}' { MPRINT("{ stml ret } -> fblock"); $$ = $$ = new NodeFuncBodySeq($2, $3);; }
    |   '{' ret '}'     { MPRINT("{ ret } -> fblock"); $$ = new NodeFuncBodySeq(new NodeEmptyListElem(), $2); }
 	|   '{' stml '}'	{ minc_die("function bodies must end with 'return <exp>' statement"); flerror = 1; $$ = new NodeNoop(); }
   ;

/* funcdef is a complete rule for a function definition, e.g. "list myfunction(string s) { ... }".
 */

funcdef: funcname fargl fblock	{ MPRINT("funcname fargl fblock -> funcdef");
                                    decrFunctionLevel();
									$$ = new NodeFuncDef($1, $2, $3);
								}
	;

/* methoddef is a complete rule for a struct method definition.  Looks the same as funcdef but only occurs within a struct definition.
 */

methoddef: methodname fargl fblock    { MPRINT("methodname fargl fblock -> methoddef");
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
    Node * end = new NodeNoop();    // end of the list
    Node * decl = new NodeStructDecl(idlist[0], typeName, initList);
    return new NodeSeq(end, decl);
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
double minc_memflush(double p[], int n_args)
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
