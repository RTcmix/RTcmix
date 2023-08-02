/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* symbol table management routines */

//#undef SYMBOL_DEBUG
//#undef DEBUG_SYM_MEMORY

#ifdef SYMBOL_DEBUG
#define DEBUG
static const char *dname(int x);
#endif

#include "Symbol.h"
#include "Scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include "minc_internal.h"
#include "handle.h"
#include "Node.h"
#include <RefCounted.h>

#define NO_EMALLOC_DEBUG

static Symbol *freelist = NULL;  /* free list of unused entries */

/* prototypes for local functions */
#ifdef NOTYET
static void free_node(Symbol *p);
#endif

/* Allocate and initialize and new symbol table entry for <name>. */
Symbol *	Symbol::create(const char *name)
{
	Symbol *p = freelist;
	if (p) {
		freelist = p->next;
		return new(p) Symbol(name);
	}
	else {
		return new Symbol(name);
	}
}

Symbol::Symbol(const char *symName)
    : next(NULL), _scope(-1), _name(symName)
{
#ifdef NOTYET
	defined = offset = 0;
	list = NULL;
#endif
#ifdef DEBUG_SYM_MEMORY
	DPRINT("Symbol::Symbol('%s') -> %p\n", symName, this);
#endif
}

Symbol::~Symbol()
{
#if defined(SYMBOL_DEBUG) || defined(DEBUG_SYM_MEMORY)
    DPRINT("\tSymbol::~Symbol() \"%s\" scope %d (%p)\n", _name, scope(), this);
#endif
	_scope = -1;			// we assert on this elsewhere
}

// Returns symbol for a struct's member, if present

Symbol *
Symbol::getStructMember(const char *memberName)
{
    MincStruct *mstruct = (MincStruct *)v;
    Symbol *ret = mstruct->lookupMember(memberName);
    DPRINT("Symbol::getStructMember(this=%p, member '%s') -> %p\n", this, memberName, ret);
    return ret;
}

Symbol *
Symbol::copyValue(Node *source, bool allowTypeOverwrite)
{
    DPRINT("Symbol::copyValue(this=%p, %p)\n", this, source);
#ifdef EMBEDDED
    /* Not yet handling nonfatal errors using throw/catch */
    if (source->dataType() == MincVoidType) {
        return this;
    }
#endif
    assert(_scope != -1);    // we accessed a variable after leaving its scope!
    if (dataType() != MincVoidType && source->dataType() != dataType()) {
        if (allowTypeOverwrite) {
            minc_warn("Overwriting %s variable '%s' with %s", MincTypeName(dataType()), name(), MincTypeName(source->dataType()));
        }
        else {
            minc_die("Cannot overwrite %s member '%s' with %s", MincTypeName(dataType()), name(), MincTypeName(source->dataType()));
        }
    }
    value() = source->value();
    return this;
}

// This functor object adds a new element symbol to the root struct symbol's _elements list

class ElementFun
{
public:
    ElementFun(Symbol *rootSym, MincList *memberInitList) : _root(rootSym), _initValues(NULL), _initIndex(0) {
        _initValues = (memberInitList) ? memberInitList->data : NULL;
        _initValueCount = (memberInitList) ? memberInitList->len : 0;
    }
    void operator() (const char *memberName, MincDataType type, const char *structTypename) {
        MincStruct *mstruct = (MincStruct *)_root->value();
        if (mstruct->lookupMember(memberName) != NULL) {
            minc_die("Struct contains a duplicate member '%s'", memberName);
        }
        else {
            if (_initValues != NULL) {
                if (_initIndex >= _initValueCount) {
                    minc_die("struct initializer list is missing member values");
                }
                // Initialize with provided initializer value
                const MincValue memberValue = _initValues[_initIndex++];
                // Type check.  Eventually this will be handled directly by the operator =.
                if (memberValue.dataType() != type) {
                    minc_die("struct member '%s' initialized with a %s but needs a %s", memberName, MincTypeName(memberValue.dataType()), MincTypeName(type));
                }
                mstruct->addMember(memberName, memberValue, _root->scope(), structTypename);
            }
            else {
                // Initialize with default value for type
                mstruct->addMember(memberName, MincValue(type), _root->scope(), structTypename);
            }
        }
    }
private:
    Symbol *_root;
    MincValue *_initValues;
    int _initValueCount;
    int _initIndex;
};

// The use of a "visitor" functor allows a separation between the StructType and the symbol for the actual struct instance.
// When the functor is invoked on a StructType's members, it instantiates instances of the members with proper names, types,
// etc., based on the information for each.

void Symbol::initAsStruct(const StructType *structType, MincList *initList)
{
    DPRINT("Symbol::initAsStruct(this=%p, structType=%p) - creating struct and adding all members\n", this, structType);
    this->v = MincValue(new MincStruct(structType->name()));
    ElementFun functor(this, initList);
    structType->forEachMember(functor);
}

static struct str {             /* string table */
    char *str;                   /* string */
    struct str *next;            /* next entry */
} *stab[HASHSIZE] = {
    0
};

/* Lookup <str> and install if necessary; return pointer. */
char *
strsave(const char *str)
{
    int h;
    struct str *p;
    
    h = hash(str);
    for (p = stab[h]; p != NULL; p = p->next)
        if (strcmp(str, p->str) == 0)
            return (p->str);
    p = (struct str *) emalloc(sizeof(struct str));
    if (p == NULL)
        return NULL;
    p->str = (char *) emalloc(strlen(str) + 1);
    if (p->str == NULL) {
        efree(p);
        return NULL;
    }
    strcpy(p->str, str);
    p->next = stab[h];
    stab[h] = p;
    
#ifdef SYMBOL_DEBUG
    DPRINT("strsave ('%s') => %p\n", str, p);
#endif
    return p->str;
}

void
free_symbols()
{
	rtcmix_debug("free_symbols", "freeing scopes, symbols and string tables\n");
    free_scopes();
	for (int s = 0; s < HASHSIZE; ++s) {
		struct str *str;
		for (str = stab[s]; str != NULL; ) {
			struct str *next = str->next;
			free(str->str);
			free(str);
			str = next;
		}
		stab[s] = NULL;
	}
#ifdef SYMBOL_DEBUG
	rtcmix_print("free_symbols done\n");
#endif
}

#ifdef NOTYET
/* Free storage for reuse.  Very closely connected to symalloc.
   TBD:  only allow a maximum freelist length
*/
static void
free_node(Symbol *p)
{
   if (p == NULL) {
      minc_warn("free_node was called with NULL ptr ");
      return;
   }

   if (freelist == NULL)
      freelist = p;
   else {
      p->next = freelist;
      freelist = p;
   }
}
#endif

void
Symbol::print(const char *spacer)
{
#ifdef SYMBOL_DEBUG
    DPRINT("%sSymbol %p: '%s', scope: %d, type: %s\n", spacer, this, name(), scope(), dname(dataType()));
#endif
}

#ifdef SYMBOL_DEBUG

/* Return string representation of type or scope. */
static const char *
dname(int x)
{
   static struct tname {
      int val;
      const char *name;
   } tnames[] = {
//      { MincIntType, "int" },
      { MincFloatType, "float" },
      { MincHandleType, "handle" },
      { MincListType, "list" },
      { MincMapType, "map" },
      { MincStringType, "string" },
      { MincStructType, "struct" },
      { MincFunctionType, "function" },
      { MincVoidType, "void" },
      { 0, 0 }
   };
   static char buf[30];
   int i;

   for (i = 0; tnames[i].name; i++)
      if (tnames[i].val == x)
         return (tnames[i].name);
   sprintf(buf, "<%d>", x);
   return buf;
}
#endif

