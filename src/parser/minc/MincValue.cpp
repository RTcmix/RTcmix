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
#include "minc_handle.h"
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
#ifdef DEBUG_MINC_MEMORY
    MPRINT("MincList::MincList: %p alloc'd with len %d\n", this, inLen);
#endif
}

MincList::~MincList()
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("deleting MincList %p\n", this);
#endif
    if (data != NULL) {
#ifdef DEBUG_MINC_MEMORY
        MPRINT("deleting MincList data %p...\n", data);
#endif
        delete [] data;
        data = NULL;
    }
#ifdef DEBUG_MINC_MEMORY
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

bool MincList::removeAtIndex(int itemIndex)
{
    MincValue *oldList = data;
    int newLen = len - 1;
    data = new MincValue[newLen];
    int i;
    for (i = 0; i < itemIndex; ++i) {
        data[i] = oldList[i];
    }
    for (; i < newLen; i++) {
        data[i] = oldList[i + 1];
    }
    len = newLen;
    delete [] oldList;
    return true;
}

bool MincList::insertAtIndex(const MincValue &item, int itemIndex)
{
    MincValue *oldList = data;
    int newLen = std::max(len + 1, itemIndex + 1);
    data = new MincValue[newLen];
    int i;
    // Copy items from before insert
    for (i = 0; i < itemIndex && i < len; ++i) {
        data[i] = oldList[i];
    }
    // Pad if necessary with 0's to match other list functionality
    for (; i < itemIndex; ++i) {
        data[i] = MincValue(0.0);
    }
    data[itemIndex] = item;
    // Copy items after the insert
    for (i = itemIndex+1; i <= len; ++i) {
        data[i] = oldList[i-1];
    }
    len = newLen;
    delete [] oldList;
    return true;
}

bool
MincList::operator == (const MincList &rhs)
{
    if (len != rhs.len) {
        return false;   // different lengths always unequal
    }
    else if (len == 0 && rhs.len == 0) {
        return true;    // empty lists always equal
    }
    bool same = false;
    for (int i = 0; i < len; ++i) {
        same = (data[i] == rhs.data[i]);
        if (!same) {
            return false;
        }
    }
    return same;
}

bool
MincList::operator < (const MincList &rhs)
{
    if (len > rhs.len) {
        return false;   // shorter always less
    }
    else if (len < rhs.len) {
        return true;
    }
    // At this point, lengths are equal
    bool isLess = false;
    int i = 0;
    for (; i < len-1; ++i) {
        isLess = (data[i] < rhs.data[i]);
        if (isLess) {
            return true;
        }
        else if (data[i] > rhs.data[i]) {
            return false;
        }
    }
    // last pair must be <
    return (data[i] < rhs.data[i]);
}

bool
MincList::operator > (const MincList &rhs)
{
    if (len > rhs.len) {
        return true;
    }
    else if (len < rhs.len) {
        return false;
    }
    // At this point, lengths are equal
    bool isGreater = false;
    int i = 0;
    for (; i < len-1; ++i) {
        isGreater = (data[i] > rhs.data[i]);
        if (isGreater) {
            return true;
        }
        else if (data[i] < rhs.data[i]) {
            return false;
        }
    }
    return (data[i] > rhs.data[i]);
}

MincMap::MincMap()
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("MincMap::MincMap: %p alloc'd\n", this);
#endif
}

MincMap::~MincMap()
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("deleting MincMap %p\n", this);
#endif
}

bool MincMap::contains(const MincValue &element)
{
    return map.count(element) > 0;
}

bool  MincMap::remove(const MincValue &element)
{
    return map.erase(element) != 0;
}

bool
MincMap::MincValueCmp::operator()(const MincValue& lhs, const MincValue& rhs) const
{
    return lhs < rhs;
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

// addMember() is called during the instantiation of a struct-type object.  It creates a unique symbol for this
// newly-instantiated member of the struct

Symbol * MincStruct::addMember(const char *name, const MincValue &value, int scope, const char *structTypeName)
{
    // RIGHT HERE, WE NEED THE FOLLOWING LINES, BUT WE ALSO NEED TO BE PASSED THE STRUCT TYPENAME
    const StructType *structType = NULL;
    if (value.dataType() == MincStructType) {
        structType = lookupStructType(structTypeName, GlobalLevel);    // GlobalLevel for now
        if (!structType) {
            minc_die("struct type '%s' is not defined", structTypeName);
        }
    }

    // Element symbols are not linked to any scope, so we call create() directly.
    Symbol *memberSym = Symbol::create(name);
    DPRINT("MincStruct::addMember(member '%s', type %s) => symbol %p\n", name, MincTypeName(value.dataType()), memberSym);
    memberSym->setValue(value);     // initialize member value
    memberSym->_scope = scope;
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

MincFunction::MincFunction(Node *argumentList, Node *functionBody, MincFunction::Type type)
    : _argumentList(argumentList), _functionBody(functionBody), _type(type)
{
    ENTER();
    _argumentList->ref();
    _functionBody->ref();
}

MincFunction::~MincFunction()
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("deleting MincFunction %p\n", this);
#endif
    _functionBody->unref();
    _argumentList->unref();
}

void
MincFunction::handleThis(MincStruct *structForThis, const char *thisName)
{
    if (_type == Method) {
        Node *nodeStructDecl = new NodeStructDecl(strsave("this"), structForThis->typeName());
        nodeStructDecl->ref();
        TPRINT("MincFunction::handleThis: declaring symbol for 'this' from called object '%s'\n",
               thisName ? thisName : "temp");
        Node *declaredVarThis = nodeStructDecl->exct();
        declaredVarThis->setValue(MincValue(structForThis));  // NOTE: HOW DO I PROTECT AGAINS type override
        TPRINT("MincFunction::handleThis: copying source symbol's value(s) into symbol for 'this'\n");
        declaredVarThis->symbol()->setValue(MincValue(structForThis));
        nodeStructDecl->unref();
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
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincHandle)\n", this);
#endif
    _u.handle = h; ref_handle(h);
}

MincValue::MincValue(MincList *l) : type(MincListType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincList *)\n", this);
#endif
    _u.list = l; RefCounted::ref(l);
}

MincValue::MincValue(MincMap *m) : type(MincMapType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincMap *)\n", this);
#endif
    _u.map = m; RefCounted::ref(m);
}

MincValue::MincValue(MincStruct *str) : type(MincStructType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincStruct *)\n", this);
#endif
    _u.mstruct = str; RefCounted::ref(str);
}

MincValue::MincValue(MincFunction *func) : type(MincFunctionType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincFunction *)\n", this);
#endif
    _u.mfunc = func; RefCounted::ref(func);
}

MincValue::MincValue(MincDataType inType) : type(inType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for MincDataType)\n", this);
#endif
    _u.list = NULL;        // to zero our contents
}

MincValue::MincValue(const MincValue &rhs) : type(MincVoidType)
{
#ifdef DEBUG_MINC_MEMORY
    MPRINT("created MincValue %p (for copy ctor with rhs %p)\n", this, &rhs);
#endif
    *this = rhs;
}

MincValue::~MincValue()
{
#ifdef DEBUG_MINC_MEMORY
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
#ifdef DEBUG_MINC_MEMORY
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

void MincValue::print() const
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
#ifdef DEBUG_MINC_MEMORY
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

const MincValue& MincValue::operator = (MincFunction *f)
{
    RefCounted::ref(f);    // ref before unref
    doClear();
    type = MincFunctionType;
    _u.mfunc = f;
    return *this;
}

const MincValue& MincValue::operator += (const MincValue &rhs)
{
    throw UnimplementedException("MincValue::operator +=");
    return *this;
}

const MincValue& MincValue::operator -= (const MincValue &rhs)
{
    throw UnimplementedException("MincValue::operator -=");
    return *this;
}

const MincValue& MincValue::operator *= (const MincValue &rhs)
{
    throw UnimplementedException("MincValue::operator *=");
    return *this;
}

const MincValue& MincValue::operator /= (const MincValue &rhs)
{
    throw UnimplementedException("MincValue::operator /=");
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
    throw UnimplementedException("MincValue::operator[]");
    return _u.list->data[iIndex];
}

bool MincValue::operator == (const MincValue &rhs) const
{
    // Allow a zero check
    if (rhs.isZero()) {
        return rawValue() == 0ULL;
    }
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("Attempt to compare variables having different types"));
    switch (type) {
        case MincFloatType:
            return cmp(_u.number, rhs._u.number) == 0;
        case MincStringType:
            return same(_u.string, rhs._u.string);
        case MincHandleType:
        case MincFunctionType:
        case MincMapType:
        case MincStructType:
            return (rawValue() == rhs.rawValue());
        case MincListType:
            return (_u.list == NULL || rhs._u.list == NULL) ?
                _u.list == rhs._u.list : *_u.list == *rhs._u.list;
       default:
            throw InvalidTypeException("Can't compare objects of this type");
    }
}

bool MincValue::operator != (const MincValue &rhs) const
{
    return !(*this == rhs);
}

bool MincValue::operator < (const MincValue &rhs) const
{
    if (rhs.type != this->type) {
        return rawValue() < rhs.rawValue();
    }
    switch (type) {
        case MincFloatType:
            return cmp(_u.number, rhs._u.number) == -1;
        case MincStringType:
            return smaller(_u.string, rhs._u.string);
        case MincHandleType:
        case MincFunctionType:
            return rawValue() < rhs.rawValue();
        case MincListType:
            // This logic handles either side being NULL
            return (_u.list == NULL && rhs._u.list == NULL) ? false :
                (_u.list == NULL) ? true : (rhs._u.list == NULL) ? false : *_u.list < *rhs._u.list;
        case MincMapType:
        case MincStructType:
       default:
            throw InvalidTypeException("Can't compare objects of this type");
    }
}

bool MincValue::operator > (const MincValue &rhs) const
{
    if (rhs.type != this->type) {
        return (rawValue() > rhs.rawValue());
    }
    switch (type) {
        case MincFloatType:
            return cmp(_u.number, rhs._u.number) == 1;
        case MincStringType:
            return bigger(_u.string, rhs._u.string);
        case MincHandleType:
        case MincFunctionType:
            return (rawValue() > rhs.rawValue());
        case MincListType:
            // This logic handles either side being NULL
            return (_u.list == NULL && rhs._u.list == NULL) ? false :
                (_u.list == NULL) ? false : (rhs._u.list == NULL) ? true : *_u.list > *rhs._u.list;
        case MincMapType:
        case MincStructType:
       default:
            throw InvalidTypeException("Can't compare objects of this type");
    }
}

bool MincValue::operator <= (const MincValue &rhs) const
{
    return !(*this > rhs);
}

bool MincValue::operator >= (const MincValue &rhs) const
{
    return !(*this < rhs);
}
