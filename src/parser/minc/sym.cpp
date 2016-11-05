/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#undef SYMBOL_DEBUG
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
	Symbol *install(const char *name);
	Symbol *lookup(const char *name);
	int		depth() const { return _depth; }
	void	dump();
protected:
	virtual ~Scope();
private:
	int		_depth;			  /* for debugging */
	Symbol *htab[HASHSIZE];   /* hash table */
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
Scope::lookup(const char *name)
{
	Symbol *p = NULL;
	
	for (p = htab[hash(name)]; p != NULL; p = p->next)
		if (name == p->name())
			break;
	
	DPRINT("Scope::lookup (%p, '%s') [scope %d] => %p\n", this, name, depth(), p);
	return p;
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

static ScopeStack *sScopeStack = NULL;
static CallStack *sCallStack = NULL;

class InitializeScope
{
public:
	InitializeScope() {
		sScopeStack = new std::vector<Scope *>;
		// We don't use push_scope() here because of the asserts.
		Scope *globalScope = new Scope(0);
		globalScope->ref();
		sScopeStack->push_back(globalScope);
	}
};

#ifdef STANDALONE
static InitializeScope sInitializeScope;
#define CHECK_SCOPE_INIT() while(0)
#else
#define CHECK_SCOPE_INIT() if (sScopeStack == NULL) do { InitializeScope(); } while (0)
#endif

void push_scope()
{
	assert(!sScopeStack->empty());
	int newscope = current_scope() + 1;
	Scope *scope = new Scope(newscope);
	DPRINT("push_scope() => %d (sScopeStack %p) added scope %p\n", newscope, sScopeStack, scope);
	scope->ref();
	sScopeStack->push_back(scope);
}

void pop_scope() {
	DPRINT("pop_scope() => %d (sScopeStack %p)\n", current_scope()-1, sScopeStack);
	assert(!sScopeStack->empty());
	Scope *top = sScopeStack->back();
	sScopeStack->pop_back();
	assert(!sScopeStack->empty());
	top->unref();
	DPRINT("pop_scope done\n");
}

int current_scope()
{
	CHECK_SCOPE_INIT();
	int current = (int) sScopeStack->size() - 1;
	return current;
}

void restore_scope(int scope)
{
	DPRINT("restore_scope() => %d\n", scope);
	while (current_scope() > scope) {
		Scope *top = sScopeStack->back();
		sScopeStack->pop_back();
		top->unref();
	}
	DPRINT("restore_scope done\n");
}

void clear_scope_stack(ScopeStack *stack)
{
	while (!stack->empty()) {
		Scope *top = stack->back();
		stack->pop_back();
		top->unref();
	}
}

/* CallStack code.  Whenever we call a user-defined function, we create and push a
 * new ScopeStack into the CallStack and make this scope stack the current one.  We
 * copy the global (level 0) scope into each new ScopeStack.
 */

void push_function_stack()
{
	DPRINT("push_function_stack()\n");
	// Lazy init:  Most people will never use this
	if (sCallStack == NULL) {
		sCallStack = new CallStack;
	}
	DPRINT("pushing sScopeStack %p\n", sScopeStack);
	sCallStack->push_back(sScopeStack);
	ScopeStack *newStack = new ScopeStack;
	Scope *globalScope = sScopeStack->front();
	globalScope->ref();
	newStack->push_back(globalScope);
	sScopeStack = newStack;
	DPRINT("sScopeStack now %p\n", sScopeStack);
	dump_symbols();
}

void pop_function_stack()
{
	DPRINT("pop_function_stack()\n");
	assert(sCallStack != NULL);
	assert(!sCallStack->empty());
	DPRINT("destroying stack %p\n", sScopeStack);
	clear_scope_stack(sScopeStack);
	delete sScopeStack;
	sScopeStack = sCallStack->back();
	sCallStack->pop_back();
	DPRINT("sScopeStack now %p\n", sScopeStack);
	assert(sScopeStack != NULL);
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

Symbol::Symbol(const char *symName) : next(NULL), scope(-1), _name(symName), node(NULL)
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
	delete node;
	node = NULL;
	scope = -1;			// we assert on this elsewhere
}

void
free_symbols()
{
#ifdef SYMBOL_DEBUG
	rtcmix_print("freeing scopes, symbols and string tables...\n");
#endif
	// We should not be stuck in a function call.
	assert(sCallStack == NULL || sCallStack->size() == 0);
	delete sCallStack;
	sCallStack = NULL;
	clear_scope_stack(sScopeStack);
	delete sScopeStack;
#ifdef EMBEDDED
	// Parsing may continue after call to free symbols, so...
	InitializeScope();
#else
	sScopeStack = NULL;
#endif
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
	rtcmix_print("done\n");
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


/* Allocate a new entry for name and install it. */
Symbol *
install(const char *name, Bool isGlobal)
{
	CHECK_SCOPE_INIT();
	return isGlobal ? sScopeStack->front()->install(name) : sScopeStack->back()->install(name);
}

/* Lookup <name> at a given scope; return pointer to entry.
   Will match for smallest scope that is >= than that requested.
   So, looking for GLOBAL will not find LOCAL or PARAM.
   If scope is OR'd with S_ANY, lookup will return the first symbol
   matching the name, regardless of scope.
 */
/* WARNING: it can only find symbol if name is a ptr returned by strsave */
Symbol *
lookup(const char *name, LookupType lookupType)
{
	CHECK_SCOPE_INIT();
	Symbol *p = NULL;
	int foundLevel = -1;
	const char *typeString;
	if (lookupType == AnyLevel) {
		typeString = "AnyLevel";
		// Start at deepest scope and work back to global
		for (std::vector<Scope *>::reverse_iterator it = sScopeStack->rbegin(); it != sScopeStack->rend(); ++it) {
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
		if ((p = sScopeStack->front()->lookup(name)) != NULL) {
			foundLevel = sScopeStack->front()->depth();
		}
	}
	else if (lookupType == ThisLevel) {
		typeString = "ThisLevel";
		// Current scope only
		if ((p = sScopeStack->back()->lookup(name)) != NULL) {
			foundLevel = sScopeStack->back()->depth();
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
	CHECK_SCOPE_INIT();
	Symbol *sym = lookup(name, ThisLevel);	// Check at current scope *only*
	if (sym != NULL) {
		DPRINT("\tfound it at same scope\n");
		return sym;
	}
	else {
		DPRINT("\tnot in this scope - trying others\n");
		sym = lookup(name, AnyLevel);
		if (sym) {
			DPRINT("\tfound it\n");
		}
		else {
			DPRINT("\tnot found - installing %s\n", inFunctionCall ? "at current scope" : "at global scope");
		}
		return (sym) ? sym : install(name, inFunctionCall ? NO : YES);
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
		DPRINT("ScopeStack %p:\n", sScopeStack);
		for (ScopeStack::iterator it2 = sScopeStack->begin(); it2 != sScopeStack->end(); ++it2) {
			Scope *scope = *it2;
			DPRINT("    Scope %p [%d]:\n", scope, scope->depth());
			scope->dump();
		}
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


