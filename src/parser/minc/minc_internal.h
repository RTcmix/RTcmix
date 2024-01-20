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
#include <handle.h>
#include "rename.h"
#include "minc_defs.h"
#include "RefCounted.h"
#include <vector>
#include <string.h>
#include <stdio.h>      // snprintf
#include <new>          // std::bad_alloc

#ifdef DEBUG
   #define DPRINT(...) rtcmix_print(__VA_ARGS__)
#else
   #define DPRINT(...)
#endif

/* important Minc tuning parameters */
#define YYLMAX   2048      /* maximum yacc line length */
#define MAXSTACK 15        /* depth of function call or list recursion */
#define HASHSIZE 107       /* number of buckets in string table */
#define MAX_MESSAGE_SIZE 1024

enum MincWarningLevel {
    MincNoWarnings = 0,
    MincNoDefaultedArgWarnings = 1,
    MincAllWarnings = 2,
    MincWarningsAsErrors = 5
};

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
	OpGreaterEqual,
    OpPlusPlus,
    OpMinusMinus
} OpKind;

/* prototypes for internal Minc use */

#define EPSILON DBL_EPSILON

/* error.cpp */
void sys_error(const char *msg);
void minc_advise(const char *msg, ...);
void minc_warn(const char *msg, ...);
void minc_die(const char *msg, ...);
void minc_internal_error(const char *msg, ...);
extern char *concat_error_message(char *outbuf, int maxLen, const char *message, ...);

extern "C" void yyerror(const char *msg);
extern "C" const char *yy_get_current_include_filename();

#define minc_try try
#define minc_catch(actions) catch(...) { if (true) { actions } throw; }

class RTException
{
public:
	RTException(const char *msg) {
        concat_error_message(_mesg, MAX_MESSAGE_SIZE, msg);
    }
    RTException(const RTException &rhs) { strcpy(_mesg, rhs.mesg()); }
	const char *mesg() const { return _mesg; }
private:
	char _mesg[MAX_MESSAGE_SIZE];
};


class RTFatalException : public RTException
{
public:
	RTFatalException(const char *msg) : RTException(msg) {}
};

// Code that has not been finished (DAS HACK)

class UnimplementedException : public RTFatalException
{
public:
    UnimplementedException(const char *msg) : RTFatalException(msg) {}
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

// Such as divide or mod by zero

class ArithmaticException : public RTFatalException
{
public:
    ArithmaticException(const char *msg) : RTFatalException(msg) {}
};

class MincObject
{
public:
    void *operator new(size_t size);
    void operator delete(void *);
};

typedef double MincFloat;
typedef const char *MincString;
typedef void *MincHandle;  // contents of this is opaque to Minc

enum MincDataType {
    MincVoidType        = 0,
    MincFloatType       = 1,       /* a floating point number, either float or double */
    MincStringType      = 2,
    MincHandleType      = 4,
    MincListType        = 8,
    MincMapType         = 16,
    MincStructType      = 32,
    MincFunctionType    = 64   /* a callable object */
};

class MincValue;
class MincList;
class MincMap;
class Node;

union YYSTYPE {
    int ival;
    Node *node;
    char *str;
};
#define YYSTYPE_IS_DECLARED   /* keep bison from declaring YYSTYPE as an int */

enum ScopeLookupType { AnyLevel = 0, GlobalLevel = 1, ThisLevel = 2 };

void printargs(const char *funcname, const Arg arglist[], const int nargs);

char *strsave(const char *str);
void clear_elem(MincValue *);
void unref_value_list(MincValue *);

/* utils.cpp */
int is_float_list(const MincList *list);
MincFloat *float_list_to_array(const MincList *list);
MincList *array_to_float_list(const MincFloat *array, const int len);
const char *MincTypeName(MincDataType type);

int hash(const char *c);
int cmp(MincFloat f1, MincFloat f2);

// returns true if both are null or strings are identical
// Logic: if both are null, true.  If both are not  null, compare.  Else false;

inline bool same(MincString s1, MincString s2)
{
    return (s1 == s2) ? true : (s1 != NULL && s2 != NULL) ? strcmp(s1, s2) == 0 : false;
}

inline bool bigger(MincString s1, MincString s2)
{
    return (s1 == s2) ? false :
                        (s1 != NULL && s2 != NULL) ?
                            strcmp(s1, s2) > 0 :
                                (s1 != NULL) ? true : false;
}

inline bool smaller(MincString s1, MincString s2)
{
    return (s1 == s2) ? false :
                        (s1 != NULL && s2 != NULL) ?
                            strcmp(s1, s2) < 0 :
                                (s1 == NULL) ? true : false;
}

char *emalloc(long nbytes);
void efree(void *mem);

inline void *	MincObject::operator new(size_t size)
{
	char *mem = emalloc(size);
    if (!mem) {
        throw std::bad_alloc();
    }
    return mem;
}

inline void	MincObject::operator delete(void *ptr)
{
	efree(ptr);
}

#endif /* _MINC_INTERNAL_H_ */
