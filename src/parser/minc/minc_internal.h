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
#include "minc_defs.h"
#include "utils.h"

#ifdef DEBUG
   #define DPRINT(...) rtcmix_print(__VA_ARGS__)
#else
   #define DPRINT(...)
#endif

/* important Minc tuning parameters */
#define YYLMAX   2048      /* maximum yacc line length */
#define MAXSTACK 15        /* depth of function call or list recursion */
#define HASHSIZE 107       /* number of buckets in string table */

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

/* prototypes for internal Minc use */

typedef double MincFloat;
#define EPSILON DBL_EPSILON

typedef const char *MincString;
typedef void *MincHandle;  // contents of this is opaque to Minc

/* error.cpp */
void sys_error(const char *msg);
void minc_advise(const char *msg, ...);
void minc_warn(const char *msg, ...);
void minc_die(const char *msg, ...);
void minc_internal_error(const char *msg, ...);
extern "C" void yyerror(const char *msg);
#ifdef EMBEDDED
// These are used to determine if parser should bail out (since it never exits)
void set_rtcmix_error(int err);
Bool was_rtcmix_error();
#else
#define set_rtcmix_error(x)
#define was_rtcmix_error() 0
#endif

class MincObject
{
public:
	void *operator new(size_t size);
	void operator delete(void *);
};

class Node;

typedef union {
   int ival;
   Node *node;
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

/* A MincList contains an array of MincListElem's, whose underlying data
   type is flexible.  So a MincList is an array of arbitrarily mixed types
   (any of the types represented in the MincDataType enum), and it can
   support nested lists.
*/

class MincList;

union MincValue {
   MincFloat number;
   MincString string;
   MincHandle handle;
   MincList *list;
};

class MincListElem : public MincObject
{
public:
	MincListElem() : type(MincVoidType) { val.number = 0.0; }
	MincListElem(MincFloat f) : type(MincFloatType) { val.number = f; }
	MincListElem(MincString s) : type(MincStringType) { val.string = s; }
	MincListElem(MincHandle h) : type(MincHandleType) { val.handle = h; }	// TODO: reference?
	~MincListElem() {}
	
	MincDataType	dataType() const { return type; }
// private:				TODO: FINISH MAKING INTO REAL CLASS
   MincDataType type;
   MincValue val;
};

class MincList : public MincObject
{
public:
	MincList(int len);
	~MincList();
	void resize(int newLen);
	int len;                /* number of MincListElem's in <data> array */
	int refcount;			/* reference count for contained data */
	MincListElem *data;
};

class Node;

class Symbol {       		/* symbol table entries */
public:
	static Symbol *	create(const char *name);
	Symbol(const char *name);
	~Symbol();
	Symbol *next;       		  /* next entry on hash chain */
	int scope;
	MincDataType type;         /* type of data represented by symbol */
	const char *name;          /* symbol name */
	MincValue v;
	Node *node;		  		/* for symbols that are functions, function def */
#ifdef NOTYET
	short defined;             /* set when function defined */
	short offset;              /* offset in activation frame */
	Symbol *plist;             /* next parameter in parameter list */
#endif
};

/* builtin.cpp */
int call_builtin_function(const char *funcname, const MincListElem arglist[],
						  const int nargs, MincListElem *retval);

/* callextfunc.cpp */
int call_external_function(const char *funcname, const MincListElem arglist[],
						   const int nargs, MincListElem *return_value);
MincHandle minc_binop_handle_float(const MincHandle handle, const MincFloat val, OpKind op);
MincHandle minc_binop_float_handle(const MincFloat val, const MincHandle handle, OpKind op);
MincHandle minc_binop_handles(const MincHandle handle1, const MincHandle handle2, OpKind op);

/* sym.cpp */
void push_function_stack();
void pop_function_stack();
void push_scope();
void pop_scope();
int current_scope();
void restore_scope(int scope);
Symbol *install(const char *name, Bool isGlobal);
enum LookupType { AnyLevel = 0, GlobalLevel = 1, ThisLevel = 2 };
Symbol *lookup(const char *name, LookupType lookupType);
Symbol * lookupOrAutodeclare(const char *name, Bool inFunctionCall);
char *strsave(const char *str);
char *emalloc(long nbytes);
void efree(void *mem);
void clear_elem(MincListElem *);
void unref_value_list(MincValue *);
void free_symbols();
void dump_symbols();

/* utils.cpp */
int is_float_list(const MincList *list);
MincFloat *float_list_to_array(const MincList *list);
MincList *array_to_float_list(const MincFloat *array, const int len);
const char *MincTypeName(MincDataType type);

inline void *	MincObject::operator new(size_t size)
{
	return emalloc(size);
}

inline void	MincObject::operator delete(void *ptr)
{
	efree(ptr);
}

#endif /* _MINC_INTERNAL_H_ */
