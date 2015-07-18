/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#undef SYMBOL_DEBUG
#undef DEBUG_MEMORY

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
#include <RefCounted.h>

#define NO_EMALLOC_DEBUG

static struct str {             /* string table */
   char *str;                   /* string */
   struct str *next;            /* next entry */
} *stab[HASHSIZE] = {
   0
};

static struct symbol *freelist = NULL;  /* free list of unused entries */

/* prototypes for local functions */
static struct symbol *symalloc(const char *name);
static void free_symbol(struct symbol *p);
#ifdef NOTYET
static void free_node(struct symbol *p);
#endif
#ifdef NOMORE
static char *dname(int x);
static void dump(struct symbol *p, FILE * fp);
#endif
static int hash(const char *s);

// New Scope Code

class Scope : public RefCounted {
public:
	Scope(int inDepth) : _depth(inDepth) { memset(htab, 0, sizeof(htab)); }
	Symbol *install(const char *name);
	Symbol *lookup(const char *name);
	int		depth() const { return _depth; }
protected:
	virtual ~Scope();
private:
	int		_depth;			  /* for debugging */
	Symbol *htab[HASHSIZE];   /* hash table */
};

Scope::~Scope()
{
#ifdef DEBUG_MEMORY
	DPRINT("Scope::~Scope(%p)\n", this);
#endif
	for (int s = 0; s < HASHSIZE; ++s) {
		for (Symbol *p = htab[s]; p != NULL; ) {
			Symbol *next = p->next;
			free_symbol(p);
			p = next;
		}
		htab[s] = NULL;
	}
}

Symbol *
Scope::install(const char *name)
{
	Symbol *p = symalloc(name);
	int h = hash(name);
	p->next = htab[h];
	p->scope = depth();
	p->type = MincVoidType;
	htab[h] = p;
	p->v.number = 0.0;
	
#ifdef SYMBOL_DEBUG
	DPRINT("Scope::install ('%s') => %p [scope %d]\n", name, p, p->scope);
#endif
	return p;
}

Symbol *
Scope::lookup(const char *name)
{
	Symbol *p = NULL;
	
	for (p = htab[hash(name)]; p != NULL; p = p->next)
		if (name == p->name)
			break;
	
	DPRINT("Scope::lookup ('%s') [scope %d] => %p\n", name, depth(), p);
	return p;
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
	DPRINT("push_scope() => %d (sScopeStack %p)\n", newscope, sScopeStack);
	Scope *scope = new Scope(newscope);
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
}

void pop_function_stack()
{
	DPRINT("pop_function_stack()\n");
	assert(sCallStack != NULL);
	assert(!sCallStack->empty());
	DPRINT("destroying stack %p\n", sScopeStack);
	delete sScopeStack;
	sScopeStack = sCallStack->back();
	sCallStack->pop_back();
	DPRINT("sScopeStack now %p\n", sScopeStack);
	assert(sScopeStack != NULL);
}

/* Allocate and initialize and new symbol table entry for <name>. */
static struct symbol *
symalloc(const char *name)
{
   struct symbol *p;

   p = freelist;
   if (p)
      freelist = p->next;
   else {
      p = (struct symbol *) emalloc(sizeof(struct symbol));
      if (p == NULL)
         return NULL;
   }
   p->name = name;
   p->tree = NULL;
#ifdef NOTYET
   p->defined = p->offset = 0;
   p->list = NULL;
#endif
#ifdef DEBUG_MEMORY
	DPRINT("symalloc() -> %p\n", p);
#endif
   return p;
}

static void free_symbol(struct symbol *p)
{
#if defined(SYMBOL_DEBUG) || defined(DEBUG_MEMORY)
	rtcmix_print("\tfreeing symbol \"%s\" for scope %d (%p)\n", p->name, p->scope, p);
#endif
	if (p->type == MincHandleType)
		unref_handle(p->v.handle);
	else if (p->type == MincListType) {
		unref_value_list(&p->v);
	}
	if (p->tree != NULL) {
		free_tree(p->tree);		// Free the tree associated with this function symbol
	}
	free(p);
}

void
free_symbols()
{
#ifdef SYMBOL_DEBUG
	rtcmix_print("freeing symbol and string tables...\n");
#endif
	// We should not be stuck in a function call.
	assert(sCallStack == NULL || sCallStack->size() == 0);
	// Start at deepest scope (end) and work back to global (begin)
	while (!sScopeStack->empty()) {
		Scope *s = sScopeStack->back();
		sScopeStack->pop_back();
		s->unref();
	}
	for (int s = 0; s < HASHSIZE; ++s)
	{
#if 1
		struct str *str;
		for (str = stab[s]; str != NULL; ) {
			struct str *next = str->next;
			free(str->str);
			free(str);
			str = next;
		}
		stab[s] = NULL;
#endif
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
free_node(struct symbol *p)
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
struct symbol *
install(const char *name)
{
	CHECK_SCOPE_INIT();
	return sScopeStack->back()->install(name);
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
		DPRINT("lookup ('%s', %s) => %p (scope %d, type %s)\n", name, typeString, p, foundLevel, MincTypeName(p->type));
	}
	else {
		DPRINT("lookup ('%s', %s) => %p\n", name, typeString, p);
	}
#endif
   return p;
}

Symbol * lookupOrAutodeclare(const char *name)
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
		if (sym) { DPRINT("\tfound it\n"); } else { DPRINT("\tnot found - installing\n"); }
		return (sym) ? sym : install(name);		// XXX DOES THIS MATCH OLD BEHAVIOR?
	}
}

/* Lookup <str> and install if necessary; return pointer. */
char *
strsave(char *str)
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

#ifdef NOMORE
/* Return string representation of type or scope. */
static char *
dname(int x)
{
   static struct tname {
      int val;
      char *name;
   } tnames[] = {
      { T_INT, "int" },
      { T_FLOAT, "float" },
      { T_COND, "conditional" },
      { T_SCALAR, "scalar" },
      { T_FUNC, "()" },
      { T_ARRAY, "[]" },
      { S_GLOBAL, "global" },
      { S_PARAM, "parameter" },
      { S_LOCAL, "local" },
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


/* Print entire symbol table or one entry. */
static void
dump(struct symbol *p, FILE * fp)
{
   int i;

   if (fp == NULL)
      fp = stderr;
   if (p == NULL)
      for (i = 0; i < HASHSIZE; i++)
         for (p = htab[i]; p; p = p->next)
            dump(p, fp);
   else {
      fprintf(fp, "%s ", dname(p->type));
/*
   fprintf(fp, "%s%s, %sscope=%s, offset=%d\n", p->name,
   dname(xshape(p->type)), p->defined?"defined, ":"",
   dname(p->scope), p->offset);
*/
   }
}
#endif /* NOMORE */


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


