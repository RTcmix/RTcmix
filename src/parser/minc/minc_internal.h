/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* written by John Gibson, based on the classic cmix Minc by Lars Graf */

#ifndef _MINC_INTERNAL_H_
#define _MINC_INTERNAL_H_ 1

#include <float.h>   /* for epsilon constants */
#include <maxdispargs.h>
#include <ugens.h>
#include "rename.h"
#include "minc.h"

#define DEBUG
#ifdef DEBUG
   #define DPRINT(msg)                    rtcmix_print((msg))
   #define DPRINT1(msg, arg)              rtcmix_print((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       rtcmix_print((msg), (arg1), (arg2))
   #define DPRINT3(msg, arg1, arg2, arg3) rtcmix_print((msg), (arg1), (arg2), (arg3))
   #define DPRINT4(msg, arg1, arg2, arg3, arg4) \
                                 rtcmix_print((msg), (arg1), (arg2), (arg3), (arg4))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
   #define DPRINT3(msg, arg1, arg2, arg3)
   #define DPRINT4(msg, arg1, arg2, arg3, arg4)
#endif

/* important Minc tuning parameters */
#define YYLMAX   2048      /* maximum yacc line length */
#define MAXSTACK 15        /* depth of function call or list recursion */
#define HASHSIZE 107       /* number of buckets in string table */

typedef union {
   int ival;
   struct tree *trees;
   char *str;
} YYSTYPE;
#define YYSTYPE_IS_DECLARED   /* keep bison from declaring YYSTYPE as an int */

typedef enum {
   MincVoidType = 0,
   MincFloatType,       /* a floating point number, either float or double */
   MincStringType,
   MincHandleType,
   MincListType
} MincDataType;

//typedef float MincFloat;
//#define EPSILON FLT_EPSILON

typedef double MincFloat;
#define EPSILON DBL_EPSILON

typedef const char *MincString;
typedef void *MincHandle;  // contents of this is opaque to Minc

/* A MincList contains an array of MincListElem's, whose underlying data
   type is flexible.  So a MincList is an array of arbitrarily mixed types
   (any of the types represented in the MincDataType enum), and it can
   support nested lists.
*/

typedef struct {
   int len;                /* number of MincListElem's in <data> array */
   int refcount;			/* reference count for contained data */
   struct _minc_list_elem *data;
} MincList;

typedef union {
   MincFloat number;
   MincString string;
   MincHandle handle;
   MincList *list;
} MincValue;

typedef struct _minc_list_elem {
   MincDataType type;
   MincValue val;
} MincListElem;


/* scopes */
typedef enum {
   S_LOCAL =	0x1,	/* body of defined function */
   S_PARAM =	0x2,	/* argument list in defined function */
   S_GLOBAL =	0x4,	/* global scope */
   S_ANY	=	0x8		/* used during symbol install. OR'd with others */
} ScopeType;

struct tree;

typedef struct symbol {       /* symbol table entries */
   struct symbol *next;       /* next entry on hash chain */
   int scope;
   MincDataType type;         /* type of data represented by symbol */
   const char *name;          /* symbol name */
   MincValue v;
   struct tree *tree;		  /* for symbols that are functions, function def */
#ifdef NOTYET
   short defined;             /* set when function defined */
   short offset;              /* offset in activation frame */
   Symbol *plist;             /* next parameter in parameter list */
#endif
} Symbol;


/* intermediate tree representation */

typedef enum {
   NodeZero = 0,
   NodeSeq,
   NodeStore,
   NodeList,
   NodeListElem,
   NodeEmptyListElem,
   NodeSubscriptRead,
   NodeSubscriptWrite,
   NodeOpAssign,
   NodeName,
   NodeLookup,
   NodeAutoDecl,
   NodeConstf,
   NodeString,
   NodeFuncDef,
   NodeArgList,
   NodeArgListElem,
   NodeRet,
   NodeFuncSeq,
   NodeFuncCall,
   NodeCall,
   NodeAnd,
   NodeOr,
   NodeOperator,
   NodeUnaryOperator,
   NodeNot,
   NodeRelation,
   NodeIf,
   NodeWhile,
   NodeFor,
   NodeIfElse,
   NodeDecl,
   NodeFuncDecl,
   NodeBlock,
   NodeNoop
} NodeKind;

typedef enum {
   OpZero = 0,
   OpFree,
   OpPlus,
   OpMinus,
   OpMul,
   OpDiv,
   OpMod,
   OpPow,
   OpNeg,
   OpEqual,
   OpNotEqual,
   OpLess,
   OpGreater,
   OpLessEqual,
   OpGreaterEqual
} OpKind;

typedef struct tree {
   NodeKind kind;
   MincDataType type;
   OpKind op;
   union {
      struct tree *child[4];
      Symbol *symbol;
      double number;
      const char *string;
   } u;
   MincValue v;
   const char *name;              /* used for function name, symbol name (for lookup) */
} *Tree;


/* prototypes for internal Minc use */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* builtin.c */
int call_builtin_function(const char *funcname, const MincListElem arglist[],
   const int nargs, MincListElem *retval);

/* callextfunc.c */
int call_external_function(const char *funcname, const MincListElem arglist[],
   const int nargs, MincListElem *return_value);
MincHandle minc_binop_handle_float(const MincHandle handle, const MincFloat val, OpKind op);
MincHandle minc_binop_float_handle(const MincFloat val, const MincHandle handle, OpKind op);
MincHandle minc_binop_handles(const MincHandle handle1, const MincHandle handle2, OpKind op);

/* error.c */
void sys_error(const char *msg);
void minc_advise(const char *msg, ...);
void minc_warn(const char *msg, ...);
void minc_die(const char *msg, ...);
void minc_internal_error(const char *msg, ...);
void yyerror(char *msg);
#ifdef EMBEDDED
// These are used to determine if parser should bail out (since it never exits)
void set_rtcmix_error(int err);
Bool was_rtcmix_error();
#else
#define set_rtcmix_error(x)
#define was_rtcmix_error() 0
#endif

/* sym.cpp */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
void push_scope();
void pop_scope();
struct symbol *install(const char *name);
struct symbol *lookup(const char *name, Bool anyLevel);
struct symbol * lookupOrAutodeclare(const char *name);
int current_scope();
char *strsave(char *str);
char *emalloc(long nbytes);
void efree(void *mem);
void clear_elem(MincListElem *);
void unref_value_list(MincValue *);
void free_symbols();
#ifdef __cplusplus
}
#endif /* __cplusplus */

/* trees.c */
Tree tnoop(void);
Tree tseq(Tree e1, Tree e2);
Tree top(OpKind op, Tree e1, Tree e2);
Tree tunop(OpKind op, Tree e1);
Tree tbuiltinfunc(OpKind op, Tree e1);
Tree tstore(Tree e1, Tree e2);
Tree tlist(Tree e1);
Tree tlistelem(Tree e1, Tree args);
Tree temptylistelem(void);
Tree tsubscriptread(Tree e1, Tree e2);
Tree tsubscriptwrite(Tree e1, Tree e2, Tree e3);
Tree topassign(Tree e1, Tree e2, OpKind op);
Tree tname(Tree e1);
Tree tlookup(const char *symbolName);
Tree tautodecl(const char *symbolName);
Tree tstring(char *str);
Tree tconstf(MincFloat num);
Tree tcall(Tree args, char *funcname);
Tree tcand(Tree test1, Tree test2);
Tree tcor(Tree test1, Tree test2);
Tree tnot(Tree test1);
Tree trel(OpKind op, Tree e1, Tree e2);
Tree tif(Tree e1, Tree e2);
Tree tifelse(Tree e1, Tree e2, Tree e3);
Tree tfor(Tree e1, Tree e2, Tree e3, Tree e4);
Tree twhile(Tree e1, Tree e2);
Tree tfdef(Tree e1, Tree e2, Tree e3);
Tree tfcall(Tree e1, Tree e2);
Tree targlistelem(Tree e1, Tree e2);
Tree targlist(Tree e1);
Tree treturn(Tree e1);
Tree tfuncseq(Tree e1, Tree e2);
Tree tdecl(const char *name, MincDataType type);
Tree tfdecl(const char *name, MincDataType type);
Tree tblock(Tree e1);
Tree exct(Tree tp);
void free_tree(Tree tp);
void print_tree(Tree tp);
void print_symbol(struct symbol * s);

/* utils.c */
int is_float_list(const MincList *list);
MincFloat *float_list_to_array(const MincList *list);
MincList *array_to_float_list(const MincFloat *array, const int len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MINC_INTERNAL_H_ */
