/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  Created by Douglas Scott on 12/30/19.
//

#undef DEBUG

#include "MincValue.h"
#include "debug.h"
#include "handle.h"
#include "Node.h"
#include "Scope.h"
#include "Symbol.h"
#include <string.h>

/* ========================================================================== */
/* MincList */

MincList::MincList(int inLen) : len(inLen), data(NULL)
{
    ENTER();
    if (inLen > 0) {
        data = new MincValue[len];
    }
#ifdef DEBUG_MEMORY
    MPRINT("MincList::MincList: %p alloc'd with len %d\n", this, inLen);
#endif
}

MincList::~MincList()
{
#ifdef DEBUG_MEMORY
    MPRINT("deleting MincList %p\n", this);
#endif
    if (data != NULL) {
#ifdef DEBUG_MEMORY
        MPRINT("deleting MincList data %p...\n", data);
#endif
        delete [] data;
        data = NULL;
    }
#ifdef DEBUG_MEMORY
    MPRINT("\tdone\n");
#endif
}

void
MincList::resize(int newLen)
{
    MincValue *oldList = data;
    data = new MincValue[newLen];
    MPRINT("MincList %p resizing with new data %p\n", this, data);
    int i;
    for (i = 0; i < len; ++i) {
        data[i] = oldList[i];
    }
    for (; i < newLen; i++) {
        data[i] = 0.0;
    }
    len = newLen;
    delete [] oldList;
}

MincMap::MincMap()
{
#ifdef DEBUG_MEMORY
    MPRINT("MincMap::MincMap: %p alloc'd\n", this);
#endif
}

MincMap::~MincMap()
{
#ifdef DEBUG_MEMORY
    MPRINT("deleting MincMap %p\n", this);
#endif
}

bool MincMap::contains(const MincValue &element)
{
    return map.count(element) > 0;
}

bool
MincMap::MincValueCmp::operator()(const MincValue& lhs, const MincValue& rhs) const
{
    return lhs.rawValue() < rhs.rawValue();
}

#define ThrowIf(exp, excp) if (exp) { throw excp; }

/* ========================================================================== */
/* MincStruct */

MincStruct::~MincStruct()
{
    for (Symbol *member = _memberList; member != NULL; ) {
        Symbol *next = member->next;
        delete member;
        member = next;
    }
}

Symbol * MincStruct::addMember(const char *name, const MincValue &value, int scope, const char *structTypeName)
{
    // Element symbols are not linked to any scope, so we call create() directly.
    // RIGHT HERE, WE NEED THE FOLLOWING LINES, BUT WE ALSO NEED TO BE PASSED THE STRUCT TYPENAME
    const StructType *structType = NULL;
    if (value.dataType() == MincStructType) {
        structType = lookupStructType(structTypeName, GlobalLevel);    // GlobalLevel for now
        if (!structType) {
            minc_die("struct type '%s' is not defined", structTypeName);
        }
    }

    Symbol *memberSym = Symbol::create(name);
    DPRINT("MincStruct::addMember(member '%s', type %s) => symbol %p\n", name, MincTypeName(value.dataType()), memberSym);
    memberSym->value() = value;     // initialize member value
    memberSym->scope = scope;
#if ALLOW_RECURSIVE_STRUCT_INIT     /* this causes a crash if a struct contains a struct which contains a... */
    if (structType) {
        // Recursively initialize a struct member within a struct.
        memberSym->initAsStruct(structType);
    }
#endif
    // Ugly, but lets us put them on in order
    if (_memberList == NULL) {
        _memberList = memberSym;
    }
    else {
        Symbol *member;
        for (member = _memberList; member->next != NULL; member = member->next) {}
        member->next = memberSym;
    }
    return memberSym;
}

Symbol * MincStruct::lookupMember(const char *name)
{
    for (Symbol *memberSym = _memberList; memberSym != NULL; memberSym = memberSym->next) {
        if (memberSym->name() == name) {
            DPRINT("MincStruct::lookupMember('%s') => %p\n", name, memberSym);
            return memberSym;
        }
    }
    return NULL;
}

// MincStruct::print() is defined in builtin.cpp

/* ========================================================================== */
/* MincFunction */

MincFunction::MincFunction(Node *argumentList, Node *functionBody)
    : _argumentList(argumentList), _functionBody(functionBody)
{
    ENTER();
}

MincFunction::~MincFunction()
{
#ifdef DEBUG_MEMORY
    MPRINT("deleting MincFunction %p\n", this);
#endif
}

void
MincFunction::handleThis(Symbol *symbolForThis)
{
    // If NULL, it means this is a non-method function and has no 'this' argument
    if (symbolForThis) {
        MincStruct *structForThis = (MincStruct *) symbolForThis->value();
        Node *nodeStructDecl = new NodeStructDecl(strsave("this"), structForThis->typeName());
        Node *declaredVarThis = nodeStructDecl->exct();
        declaredVarThis->copyValue(symbolForThis, NO);  // dont allow type override
        DPRINT("MincFunction::handleThis: copying source symbol's value(s) into symbol for 'this'");
        declaredVarThis->symbol()->value() = symbolForThis->value();
        delete nodeStructDecl;
    }
}

void
MincFunction::copyArguments()
{
    (void)_argumentList->exct();
}

Node *
MincFunction::execute()
{
    return _functionBody->exct();
}

/* ========================================================================== */
/* MincValue */

MincValue::MincValue(MincHandle h) : type(MincHandleType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincHandle)\n", this);
#endif
    _u.handle = h; ref_handle(h);
}

MincValue::MincValue(MincList *l) : type(MincListType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincList *)\n", this);
#endif
    _u.list = l; RefCounted::ref(l);
}

MincValue::MincValue(MincMap *m) : type(MincMapType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincMap *)\n", this);
#endif
    _u.map = m; RefCounted::ref(m);
}

MincValue::MincValue(MincStruct *str) : type(MincStructType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincStruct *)\n", this);
#endif
    _u.mstruct = str; RefCounted::ref(str);
}

MincValue::MincValue(MincFunction *func) : type(MincFunctionType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincFunction *)\n", this);
#endif
    _u.mfunc = func; RefCounted::ref(func);
}

MincValue::MincValue(MincDataType inType) : type(inType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for MincDataType)\n", this);
#endif
    _u.list = NULL;        // to zero our contents
}

MincValue::MincValue(const MincValue &rhs) : type(MincVoidType)
{
#ifdef DEBUG_MEMORY
    MPRINT("created MincValue %p (for copy ctor with rhs %p)\n", this, &rhs);
#endif
    *this = rhs;
}

MincValue::~MincValue()
{
#ifdef DEBUG_MEMORY
    MPRINT("deleting MincValue %p\n", this);
#endif
    switch (type) {
        case MincHandleType:
            unref_handle(_u.handle);
            break;
        case MincListType:
            RefCounted::unref(_u.list);
            break;
        case MincMapType:
            RefCounted::unref(_u.map);
            break;
        case MincStructType:
            RefCounted::unref(_u.mstruct);
            break;
        case MincFunctionType:
            RefCounted::unref(_u.mfunc);
            break;
       default:
            break;
    }
#ifdef DEBUG_MEMORY
    MPRINT("\tdone deleting\n");
#endif
}

void MincValue::doClear()
{
    switch (type) {
        case MincFloatType:
            _u.number = 0.0;
            break;
        case MincStringType:
            _u.string = NULL;
            break;
        case MincHandleType:
            if (_u.handle != NULL) {
                MPRINT("\toverwriting existing Handle value %p\n", _u.handle);
                unref_handle(_u.handle);    // overwriting handle, so unref
                _u.handle = NULL;
            }
            break;
        case MincListType:
            if (_u.list != NULL) {
                MPRINT("\toverwriting existing MincList value %p\n", _u.list);
                RefCounted::unref(_u.list);
                _u.list = NULL;
            }
            break;
        case MincMapType:
            if (_u.map != NULL) {
                MPRINT("\toverwriting existing MincMap value %p\n", _u.map);
                RefCounted::unref(_u.map);
                _u.map = NULL;
            }
            break;
       case MincStructType:
            if (_u.mstruct != NULL) {
                MPRINT("\toverwriting existing MincStruct value %p\n", _u.mstruct);
                RefCounted::unref(_u.mstruct);
                _u.mstruct = NULL;
            }
        case MincFunctionType:
            if (_u.mfunc != NULL) {
                MPRINT("\toverwriting existing MincFunction value %p\n", _u.mfunc);
                RefCounted::unref(_u.mfunc);
                _u.mfunc = NULL;
            }
            break;
        default:
            break;
    }
}

// Note: handle, list, map, and struct elements are referenced before this call is made

void MincValue::doCopy(const MincValue &rhs)
{
    switch (rhs.type) {
        case MincFloatType:
            _u.number = rhs._u.number;
            break;
        case MincStringType:
            _u.string = rhs._u.string;
            break;
        case MincHandleType:
            _u.handle = rhs._u.handle;
            break;
        case MincListType:
            _u.list = rhs._u.list;
            break;
        case MincMapType:
            _u.map = rhs._u.map;
            break;
       case MincStructType:
            _u.mstruct = rhs._u.mstruct;
            break;
        case MincFunctionType:
            _u.mfunc = rhs._u.mfunc;
            break;
       default:
            if (type != MincVoidType) {
                MPRINT("\tAssigning from a void MincValue rhs");
            }
            break;
    }
}

bool MincValue::validType(unsigned allowedTypes) const
{
    return ((type & allowedTypes) == type);
}

void MincValue::print()
{
    switch (type) {
        case MincFloatType:
            TPRINT("%f\n", _u.number);
            break;
        case MincHandleType:
            TPRINT("handle: %p\n", _u.handle);
            break;
        case MincListType:
            TPRINT("list: %p\n", _u.list);
            break;
        case MincMapType:
            TPRINT("map: %p\n", _u.map);
            if (_u.map) {
                TPRINT("{\n");
                _u.map->print();
                TPRINT("}\n");
            }
            break;
        case MincStringType:
            TPRINT("'%s'\n", _u.string);
            break;
        case MincStructType:
            TPRINT("struct: %p\n", _u.mstruct);
            if (_u.mstruct) {
                TPRINT("{\n");
                _u.mstruct->print();
                TPRINT("}\n");
            }
            break;
        case MincFunctionType:
            TPRINT("mfunction: %p\n", _u.mfunc);
            break;
        case MincVoidType:
            TPRINT("void\n");
            break;
    }
}

// Public operators

const MincValue& MincValue::operator = (const MincValue &rhs)
{
#ifdef DEBUG_MEMORY
    ENTER();
    MPRINT("MincValue %p assigning from rhs %p)\n", this, &rhs);
#endif
    if (rhs.type == MincHandleType)
        ref_handle(rhs._u.handle);
    else if (rhs.type == MincListType)
        RefCounted::ref(rhs._u.list);
    else if (rhs.type == MincMapType)
        RefCounted::ref(rhs._u.map);
    else if (rhs.type == MincStructType)
        RefCounted::ref(rhs._u.mstruct);
    else if (rhs.type == MincFunctionType)
        RefCounted::ref(rhs._u.mfunc);
    doClear();
    type = rhs.type;
    doCopy(rhs);
    return *this;
}

const MincValue& MincValue::operator = (MincFloat f)
{
    doClear(); type = MincFloatType; _u.number = f; return *this;
}

const MincValue& MincValue::operator = (MincString s)
{
    doClear(); type = MincStringType; _u.string = s; return *this;
}

const MincValue& MincValue::operator = (MincHandle h)
{
    ref_handle(h);    // ref before unref
    doClear();
    type = MincHandleType;
    _u.handle = h;
    return *this;
}

const MincValue& MincValue::operator = (MincList *l)
{
    RefCounted::ref(l);    // ref before unref
    doClear();
    type = MincListType;
    _u.list = l;
    return *this;
}

const MincValue& MincValue::operator = (MincMap *m)
{
    RefCounted::ref(m);    // ref before unref
    doClear();
    type = MincMapType;
    _u.map = m;
    return *this;
}

const MincValue& MincValue::operator += (const MincValue &rhs)
{
    return *this;
}

const MincValue& MincValue::operator -= (const MincValue &rhs)
{
    return *this;
}

const MincValue& MincValue::operator *= (const MincValue &rhs)
{
    return *this;
}

const MincValue& MincValue::operator /= (const MincValue &rhs)
{
    return *this;
}

// RHS use

const MincValue& MincValue::operator[] (const MincValue &index) const
{
    if (!validType(MincListType)) throw InvalidTypeException("Attempting to index something that is not a list");
    if (!index.validType(MincFloatType)) throw InvalidTypeException("Index into a list must be a number");
    int iIndex = (int)(MincFloat) index;
    // FINISH ME
    return _u.list->data[iIndex];
}

// LHS use

MincValue& MincValue::operator[] (const MincValue &index)
{
    if (!validType(MincListType)) throw InvalidTypeException("Attempting to index something that is not a list");
    if (!index.validType(MincFloatType)) throw InvalidTypeException("Index into a list must be a number");
    int iIndex = (int)(MincFloat) index;
    // FINISH ME
    return _u.list->data[iIndex];
}

bool MincValue::operator == (const MincValue &rhs) const
{
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("attempt to compare variables having different types"));
    switch (type) {
        case MincFloatType:
            return cmp(_u.number, rhs._u.number) == 0;
        case MincStringType:
            return strcmp(_u.string, rhs._u.string) == 0;
        default:
            throw InvalidTypeException("can't compare this type of object");
    }
}

bool MincValue::operator != (const MincValue &rhs) const
{
    return !(*this == rhs);
}

bool MincValue::operator < (const MincValue &rhs) const
{
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("attempt to compare variables having different types"));
    ThrowIf(this->type != MincFloatType, InvalidTypeException("can't compare this type of object"));
    return cmp(_u.number, rhs._u.number) == -1;
}

bool MincValue::operator > (const MincValue &rhs) const
{
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("attempt to compare variables having different types"));
    ThrowIf(this->type != MincFloatType, InvalidTypeException("can't compare this type of object"));
    return cmp(_u.number, rhs._u.number) == 1;
}

bool MincValue::operator <= (const MincValue &rhs) const
{
    return !(*this > rhs);
}

bool MincValue::operator >= (const MincValue &rhs) const
{
    return !(*this < rhs);
}
