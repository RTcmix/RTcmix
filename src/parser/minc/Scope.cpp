/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  Created by Douglas Scott on 12/30/19.
//

#include "Scope.h"
#include "RefCounted.h"
#include "Symbol.h"
#include "minc_internal.h"
#include <vector>
#include <string.h>

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
    int h = hash(name);        // TODO: FINISH COMBINING THIS INTO SYMBOL CREATOR
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
            p->dump();
    }
}

void clear_scope_stack(ScopeStack *stack)
{
    while (stack && !stack->empty()) {
        Scope *top = stack->back();
        stack->pop_back();
        top->unref();
    }
}

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

Scope * ScopeManager::globalScope() { return stack()->front(); }
Scope * ScopeManager::currentScope() { return stack()->back(); }

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
    Symbol *sym = lookupSymbol(name, ThisLevel);    // Check at current scope *only*
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

void free_scopes()
{
    // We should not be stuck in a function call.
    assert(sCallStack == NULL || sCallStack->size() == 0);
    delete sCallStack;
    sCallStack = NULL;
    ScopeManager::destroy();
}

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

/* Print entire symbol table. */

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

