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
#include "RefCounted.h"

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

class MincObject
{
public:
	void *operator new(size_t size);
	void operator delete(void *);
};

class Node;

union YYSTYPE {
   int ival;
   Node *node;
   char *str;
};
#define YYSTYPE_IS_DECLARED   /* keep bison from declaring YYSTYPE as an int */

enum MincDataType {
   MincVoidType = 0,
   MincFloatType = 1,       /* a floating point number, either float or double */
   MincStringType = 2,
   MincHandleType = 4,
   MincListType = 8
};

class RTException
{
public:
	RTException(const char *msg) : _mesg(msg) {}
	const char *mesg() { return _mesg; }
private:
	const char *_mesg;
};

class RTFatalException : public RTException
{
public:
	RTFatalException(const char *msg) : RTException(msg) {}
};

// Such as attempting to multiply two MincStrings

class UnsupportedOperationException : public RTException
{
public:
	UnsupportedOperationException(const char *msg) : RTException(msg) {}
};

class InvalidOperatorException : public RTException
{
public:
	InvalidOperatorException(const char *msg) : RTException(msg) {}
};

class NonmatchingTypeException : public RTException
{
public:
	NonmatchingTypeException(const char *msg) : RTException(msg) {}
};

// Such as indexing a MincList with a MincString

class InvalidTypeException : public RTFatalException
{
public:
	InvalidTypeException(const char *msg) : RTFatalException(msg) {}
};

class UndeclaredVariableException : public RTFatalException
{
public:
	UndeclaredVariableException(const char *msg) : RTFatalException(msg) {}
};

class ReclaredVariableException : public RTFatalException
{
public:
	ReclaredVariableException(const char *msg) : RTFatalException(msg) {}
};

/* A MincList contains an array of MincValue's, whose underlying data
   type is flexible.  So a MincList is an array of arbitrarily mixed types
   (any of the types represented in the MincDataType enum), and it can
   support nested lists.
*/

class MincValue;

class MincList : public MincObject, public RefCounted
{
public:
	MincList(int len=0);
	void resize(int newLen);
	int len;                /* number of MincValue's in <data> array */
	MincValue *data;
protected:
	virtual ~MincList();
};

class MincValue {
public:
	MincValue() : type(MincVoidType) { _u.list = NULL; }
	MincValue(MincFloat f) : type(MincFloatType) { _u.number = f; }
	MincValue(MincString s) : type(MincStringType) { _u.string = s; }
	MincValue(MincHandle h);
	MincValue(MincList *l);
	MincValue(MincDataType type);
	~MincValue();
	const MincValue& operator = (const MincValue &rhs);
	const MincValue& operator = (MincFloat f);
	const MincValue& operator = (MincString s);
	const MincValue& operator = (MincHandle h);
	const MincValue& operator = (MincList *l);

	const MincValue& operator += (const MincValue &rhs);
	const MincValue& operator -= (const MincValue &rhs);
	const MincValue& operator *= (const MincValue &rhs);
	const MincValue& operator /= (const MincValue &rhs);

	const MincValue& operator[] (const MincValue &index) const;	// for MincList access
	MincValue& operator[] (const MincValue &index);	// for MincList access

	operator MincFloat() const { return _u.number; }
	operator MincString() const { return _u.string; }
	operator MincHandle() const { return _u.handle; }
	operator MincList *() const { return _u.list; }
	
	bool operator == (const MincValue &rhs);
	bool operator != (const MincValue &rhs);
	bool operator < (const MincValue &rhs);
	bool operator > (const MincValue &rhs);
	bool operator <= (const MincValue &rhs);
	bool operator >= (const MincValue &rhs);
	
	
	
	MincDataType	dataType() const { return type; }
	void zero() { _u.list = NULL; }		// zeroes without changing type
	void print();
private:
	void doClear();
	void doCopy(const MincValue &rhs);
	bool validType(unsigned allowedTypes) const;
	MincDataType type;
	union {
		MincFloat number;
		MincString string;
		MincHandle handle;
		MincList *list;
	} _u;
};

class Node;

class Symbol {       		/* symbol table entries */
public:
	static Symbol *	create(const char *name);
	Symbol(const char *name);
	~Symbol();
	MincDataType		dataType() const { return v.dataType(); }
	const MincValue&	value() const { return v; }
	MincValue&			value() { return v; }
	const char *		name() { return _name; }
	Symbol *next;       		  /* next entry on hash chain */
	int scope;
	const char *_name;          /* symbol name */
	Node *node;		  		/* for symbols that are functions, function def */
protected:
	MincValue v;
#ifdef NOTYET
	short defined;             /* set when function defined */
	short offset;              /* offset in activation frame */
	Symbol *plist;             /* next parameter in parameter list */
#endif
};

/* builtin.cpp */
int call_builtin_function(const char *funcname, const MincValue arglist[],
						  const int nargs, MincValue *retval);

/* callextfunc.cpp */
int call_external_function(const char *funcname, const MincValue arglist[],
						   const int nargs, MincValue *return_value);
void printargs(const char *funcname, const Arg arglist[], const int nargs);
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
void clear_elem(MincValue *);
void unref_value_list(MincValue *);
void free_symbols();
void dump_symbols();

/* utils.cpp */
int is_float_list(const MincList *list);
MincFloat *float_list_to_array(const MincList *list);
MincList *array_to_float_list(const MincFloat *array, const int len);
const char *MincTypeName(MincDataType type);
void increment_score_line_offset(int offset);
int get_score_line_offset();

inline void *	MincObject::operator new(size_t size)
{
	return emalloc(size);
}

inline void	MincObject::operator delete(void *ptr)
{
	efree(ptr);
}

#endif /* _MINC_INTERNAL_H_ */
