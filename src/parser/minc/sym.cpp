/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#define SYMBOL_DEBUG
#undef DEBUG_SYM_MEMORY

#ifdef SYMBOL_DEBUG
#define DEBUG
#endif

/* symbol table management routines */
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

static struct str {             /* string table */
   char *str;                   /* string */
   struct str *next;            /* next entry */
} *stab[HASHSIZE] = {
   0
};

static Symbol *freelist = NULL;  /* free list of unused entries */

/* prototypes for local functions */
#ifdef NOTYET
static void free_node(Symbol *p);
#endif
static void dump(Symbol *p);
static int hash(const char *s);

// New Scope Code

class Scope : public RefCounted {
public:
	Scope(int inDepth) : _depth(inDepth) { memset(htab, 0, sizeof(htab)); }
	Symbol *            install(const char *name);
	Symbol *            lookup(const char *name) const;
	int                 depth() const { return _depth; }
    StructType *        installType(const char *name);
    const StructType *  lookupType(const char *name) const;
	void                dump();
protected:
	virtual             ~Scope();
private:
	int                     _depth;
    std::vector<StructType> _structTypes;   // structs declared at this scope
	Symbol *                htab[HASHSIZE];   /* hash table */
};

Scope::~Scope()
{
#ifdef DEBUG_SYM_MEMORY
	DPRINT("Scope::~Scope(%p)\n", this);
#endif
	for (int s = 0; s < HASHSIZE; ++s) {
		for (Symbol *p = htab[s]; p != NULL; ) {
			Symbol *next = p->next;
			delete p;
			p = next;
		}
		htab[s] = NULL;
	}
}

Symbol *
Scope::install(const char *name)
{
	Symbol *p = Symbol::create(name);
	int h = hash(name);		// TODO: FINISH COMBINING THIS INTO SYMBOL CREATOR
	p->next = htab[h];
	p->scope = depth();
	htab[h] = p;
	
#ifdef SYMBOL_DEBUG
	DPRINT("Scope::install (%p, '%s') => %p [scope %d]\n", this, name, p, p->scope);
#endif
	return p;
}

Symbol *
Scope::lookup(const char *name) const
{
	Symbol *p = NULL;
	
	for (p = htab[hash(name)]; p != NULL; p = p->next)
		if (name == p->name())
			break;
	
	DPRINT("Scope::lookup (%p, '%s') [scope %d] => %p\n", this, name, depth(), p);
	return p;
}

StructType *    Scope::installType(const char *name)
{
    _structTypes.push_back(StructType(name));
    return &_structTypes.back();
}

const StructType *  Scope::lookupType(const char *name) const
{
    const StructType *outType = NULL;
    for (std::vector<StructType>::const_iterator i = _structTypes.begin(); i != _structTypes.end(); ++i) {
        if ((*i).name() == name) {
            outType = &((*i));
            break;
        }
    }
    return outType;
}

void
Scope::dump()
{
	Symbol *p = NULL;
	for (int n = 0; n < HASHSIZE; ++n) {
		for (p = htab[n]; p != NULL; p = p->next)
			::dump(p);
	}
}

typedef std::vector<Scope *>ScopeStack;
typedef std::vector<ScopeStack *>CallStack;

void clear_scope_stack(ScopeStack *stack)
{
	while (stack && !stack->empty()) {
		Scope *top = stack->back();
		stack->pop_back();
		top->unref();
	}
}

class ScopeManager
{
	static ScopeStack *sScopeStack;
public:
	static ScopeStack *stack();
	static void setStack(ScopeStack *stack);
    static Scope *globalScope() { return stack()->front(); }
    static Scope *currentScope() { return stack()->back(); }
	static void dump();
	static void destroy();
};

ScopeStack *ScopeManager::sScopeStack = NULL;

ScopeStack *ScopeManager::stack()
{
	if (sScopeStack == NULL) {
		sScopeStack = new std::vector<Scope *>;
		// We don't use push_scope() here because of the asserts.
		Scope *globalScope = new Scope(0);
		globalScope->ref();
		sScopeStack->push_back(globalScope);
	}
	return sScopeStack;
}

void ScopeManager::setStack(ScopeStack *stack)
{
	sScopeStack = stack;
	DPRINT("sScopeStack now %p\n", sScopeStack);
}

void ScopeManager::destroy()
{
	DPRINT("clearing stack %p\n", sScopeStack);
	clear_scope_stack(sScopeStack);
	DPRINT("destroying stack %p\n", sScopeStack);
	delete sScopeStack;
	sScopeStack = NULL;
}

void ScopeManager::dump()
{
	// Note: this one does not auto-create the scope
	DPRINT("ScopeStack %p:\n", sScopeStack);
	for (ScopeStack::iterator it2 = sScopeStack->begin(); it2 != sScopeStack->end(); ++it2) {
		Scope *scope = *it2;
		DPRINT("    Scope %p [%d]:\n", scope, scope->depth());
		scope->dump();
	}
}

void push_scope()
{
	ScopeStack *stack = ScopeManager::stack();
	assert(!stack->empty());
	int newscope = current_scope() + 1;
	Scope *scope = new Scope(newscope);
	DPRINT("push_scope() => %d (stack %p) added scope %p\n", newscope, stack, scope);
	scope->ref();
	stack->push_back(scope);
}

void pop_scope() {
	ScopeStack *stack = ScopeManager::stack();
	DPRINT("pop_scope() => %d (stack %p)\n", current_scope()-1, stack);
	assert(!stack->empty());
	Scope *top = stack->back();
	stack->pop_back();
	assert(!stack->empty());
	top->unref();
	DPRINT("pop_scope done\n");
}

int current_scope()
{
	int current = (int) ScopeManager::stack()->size() - 1;
	return current;
}

void restore_scope(int scope)
{
	DPRINT("restore_scope() => %d\n", scope);
	ScopeStack *stack = ScopeManager::stack();
	while (current_scope() > scope) {
		Scope *top = stack->back();
		stack->pop_back();
		top->unref();
	}
	DPRINT("restore_scope done\n");
}

/* CallStack code.  Whenever we call a user-defined function, we create and push a
 * new ScopeStack into the CallStack and make this scope stack the current one.  We
 * copy the global (level 0) scope into each new ScopeStack.
 */

static CallStack *sCallStack = NULL;

void push_function_stack()
{
	DPRINT("push_function_stack()\n");
	// Lazy init:  Most people will never use this
	if (sCallStack == NULL) {
		sCallStack = new CallStack;
	}
	ScopeStack *stack = ScopeManager::stack();
    DPRINT("pushing stack %p\n", stack);
	sCallStack->push_back(stack);
	ScopeStack *newStack = new ScopeStack;
	Scope *globalScope = stack->front();
	globalScope->ref();
	newStack->push_back(globalScope);
	ScopeManager::setStack(newStack);
	dump_symbols();
}

void pop_function_stack()
{
	DPRINT("pop_function_stack()\n");
	assert(sCallStack != NULL);
	assert(!sCallStack->empty());
	ScopeManager::destroy();
	ScopeManager::setStack(sCallStack->back());
	sCallStack->pop_back();
	dump_symbols();
}

void clear_call_stack(CallStack *stack)
{
	while (!stack->empty()) {
		ScopeStack *top = stack->back();
		stack->pop_back();
		clear_scope_stack(top);
		delete top;
	}
}

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
    : next(NULL), scope(-1), _elements(NULL), _name(symName), _node(NULL)
{
#ifdef NOTYET
	defined = offset = 0;
	list = NULL;
#endif
#ifdef DEBUG_SYM_MEMORY
	DPRINT("Symbol::Symbol(%s) -> %p\n", symName, this);
#endif
}

Symbol::~Symbol()
{
#if defined(SYMBOL_DEBUG) || defined(DEBUG_SYM_MEMORY)
//	rtcmix_print("\tSymbol::~Symbol() \"%s\" for scope %d (%p)\n", name, scope, this);
#endif
	delete _node;
	_node = NULL;
	scope = -1;			// we assert on this elsewhere
}

// This functor object adds a new element symbol to the root symbol's _elements list

class ElementFun
{
public:
    ElementFun(Symbol *rootSym) : _root(rootSym) {}
    void operator() (const char *name, MincDataType type) {
        // Element symbols are not linked to any scope, so we call create() directly.
        Symbol *elementSym = Symbol::create(name);
        DPRINT("Symbol::init(element '%s') => %p\n", name, elementSym);
        elementSym->value() = MincValue(type);          // initialize MincValue to correct type for element
        elementSym->next = _root->_elements;
        _root->_elements = elementSym;
    }
private:
    Symbol *_root;
};

void Symbol::init(const StructType *structType)
{
    ElementFun functor(this);
    structType->forEachElement(functor);
}

void
free_symbols()
{
	rtcmix_debug("free_symbols", "freeing scopes, symbols and string tables\n");
	// We should not be stuck in a function call.
	assert(sCallStack == NULL || sCallStack->size() == 0);
	delete sCallStack;
	sCallStack = NULL;
	ScopeManager::destroy();
	for (int s = 0; s < HASHSIZE; ++s)
	{
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

StructType *
installType(const char *typeName, Bool isGlobal)
{
    if (ScopeManager::globalScope()->lookupType(typeName) == NULL) {
        return ScopeManager::globalScope()->installType(typeName);
    }
    else {
#ifdef EMBEDDED
        minc_warn("struct %s is already declared", typeName);
#else
        minc_die("struct %s is already declared", typeName);
#endif
        return NULL;
    }
}

const StructType *
lookupType(const char *typeName, LookupType lookupType)
{
    if (lookupType == GlobalLevel) {
        // Global scope only
        const StructType *foundType = NULL;
        ScopeStack *stack = ScopeManager::stack();
        if ((foundType = stack->front()->lookupType(typeName)) != NULL) {
            // probably print some logging here
        }
        return foundType;
    }
    else {
        minc_die("struct type lookup only supported at global scope");
        return NULL;
    }
}

/* Allocate a new entry for name and install it. */
Symbol *
installSymbol(const char *name, Bool isGlobal)
{
	return isGlobal ? ScopeManager::globalScope()->install(name) : ScopeManager::currentScope()->install(name);
}

/* Lookup <name> at a given scope; return pointer to entry.
   Will match for smallest scope that is >= than that requested.
   So, looking for GLOBAL will not find LOCAL or PARAM.
   If scope is OR'd with S_ANY, lookup will return the first symbol
   matching the name, regardless of scope.
 */
/* WARNING: it can only find symbol if name is a ptr returned by strsave */
Symbol *
lookupSymbol(const char *name, LookupType lookupType)
{
	Symbol *p = NULL;
	int foundLevel = -1;
	const char *typeString;
	ScopeStack *stack = ScopeManager::stack();
	if (lookupType == AnyLevel) {
		typeString = "AnyLevel";
		// Start at deepest scope and work back to global
		for (std::vector<Scope *>::reverse_iterator it = stack->rbegin(); it != stack->rend(); ++it) {
			Scope *s = *it;
			if ((p = s->lookup(name)) != NULL) {
				foundLevel = s->depth();
				break;
			}
		}
	}
	else if (lookupType == GlobalLevel) {
		typeString = "GlobalLevel";
		// Global scope only
		if ((p = stack->front()->lookup(name)) != NULL) {
			foundLevel = stack->front()->depth();
		}
	}
	else if (lookupType == ThisLevel) {
		typeString = "ThisLevel";
		// Current scope only
		if ((p = stack->back()->lookup(name)) != NULL) {
			foundLevel = stack->back()->depth();
		}
	}
#ifdef SYMBOL_DEBUG
	if (p) {
		DPRINT("lookup ('%s', %s) => %p (scope %d, type %s)\n", name, typeString, p, foundLevel, MincTypeName(p->dataType()));
	}
	else {
		DPRINT("lookup ('%s', %s) => %p\n", name, typeString, p);
	}
#endif
   return p;
}

Symbol * lookupOrAutodeclare(const char *name, Bool inFunctionCall)
{
	DPRINT("lookupOrAutodeclare('%s')\n", name);
	Symbol *sym = lookupSymbol(name, ThisLevel);	// Check at current scope *only*
	if (sym != NULL) {
		DPRINT("\tfound it at same scope\n");
		return sym;
	}
	else {
		DPRINT("\tnot in this scope - trying others\n");
		sym = lookupSymbol(name, AnyLevel);
		if (sym) {
			DPRINT("\tfound it\n");
		}
		else {
			DPRINT("\tnot found - installing %s\n", inFunctionCall ? "at current scope" : "at global scope");
		}
		return (sym) ? sym : installSymbol(name, inFunctionCall ? NO : YES);
	}
}

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
      { MincStringType, "string" },
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

/* Print entire symbol table or one entry. */
static void
dump(Symbol *p)
{
#ifdef SYMBOL_DEBUG
	DPRINT("        [%p] '%s', type: %s\n", p, p->name(), dname(p->dataType()));
#endif
}

void dump_symbols()
{
	DPRINT("---- SYMBOL DUMP ----\n");
	if (sCallStack != NULL) {
		DPRINT("CallStack %p:\n", sCallStack);
		for (CallStack::iterator it1 = sCallStack->begin(); it1 != sCallStack->end(); ++it1) {
			ScopeStack *stack = *it1;
			DPRINT("  ScopeStack %p:\n", stack);
			for (ScopeStack::iterator it2 = stack->begin(); it2 != stack->end(); ++it2) {
				Scope *scope = *it2;
				DPRINT("    Scope %p [%d]:\n", scope, scope->depth());
				scope->dump();
   			}
		}
	}
	ScopeManager::dump();
	DPRINT("---- END ----\n");
}

/* Has error-checking for malloc built in. */
char *
emalloc(long nbytes)
{
   char *s;

   s = (char *) malloc(nbytes);
   if (s == NULL)
      sys_error("system out of memory");

#ifndef NO_EMALLOC_DEBUG
   DPRINT("emalloc: nbytes=%d, ptr=%p\n", nbytes, s);
#endif
   return s;
}

void efree(void *mem)
{
#ifndef NO_EMALLOC_DEBUG
   DPRINT("efree: ptr=%p\n", mem);
#endif
   free(mem);
}

/* Returns an index to a hash bucket. */
static int
hash(const char *s)
{
   int i = 0;

   while (*s) {
      i = (((unsigned int) *s + i) % HASHSIZE);
      s++;
   }
   return i;
}


