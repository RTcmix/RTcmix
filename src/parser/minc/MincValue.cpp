/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  Created by Douglas Scott on 12/30/19.
//

#include "MincValue.h"
#include "debug.h"
#include "handle.h"
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
        delete []data;
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

Symbol * MincStruct::addMember(const char *name, MincDataType type, int scope)
{
    // Element symbols are not linked to any scope, so we call create() directly.
    Symbol *memberSym = Symbol::create(name);
    DPRINT("Symbol::init(member '%s') => %p\n", name, memberSym);
    memberSym->value() = MincValue(type);   // initialize MincValue to correct type for member
    memberSym->scope = scope;
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
    for (Symbol *member = _memberList; member != NULL; member = member->next) {
        if (member->name() == name) {
            return member;
        }
    }
    return NULL;
}

// MincStruct::print() is defined in builtin.cpp

/* ========================================================================== */
/* MincValue */

MincValue::MincValue(MincHandle h) : type(MincHandleType)
{
#ifdef DEBUG_MEMORY
    //    MPRINT("created MincValue %p (for MincHandle)\n", this);
#endif
    _u.handle = h; ref_handle(h);
}

MincValue::MincValue(MincList *l) : type(MincListType)
{
#ifdef DEBUG_MEMORY
    //    MPRINT("created MincValue %p (for MincList *)\n", this);
#endif
    _u.list = l; RefCounted::ref(l);
}

MincValue::MincValue(MincStruct *str) : type(MincStructType)
{
#ifdef DEBUG_MEMORY
    //    MPRINT("created MincValue %p (for MincStruct *)\n", this);
#endif
    _u.mstruct = str; RefCounted::ref(str);
}

MincValue::MincValue(MincDataType inType) : type(inType)
{
#ifdef DEBUG_MEMORY
    //    MPRINT("created MincValue %p (for MincDataType)\n", this);
#endif
    _u.list = NULL;        // to zero our contents
}

MincValue::~MincValue()
{
#ifdef DEBUG_MEMORY
    //    MPRINT("deleting MincValue %p\n", this);
#endif
    switch (type) {
        case MincHandleType:
            unref_handle(_u.handle);
            break;
        case MincListType:
            RefCounted::unref(_u.list);
            break;
        case MincStructType:
            RefCounted::unref(_u.mstruct);
            break;
        default:
            break;
    }
#ifdef DEBUG_MEMORY
    //    MPRINT("\tdone\n");
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
        case MincStructType:
            if (_u.mstruct != NULL) {
                MPRINT("\toverwriting existing MincStruct value %p\n", _u.mstruct);
                RefCounted::unref(_u.mstruct);
                _u.mstruct = NULL;
            }
            break;
        default:
            break;
    }
}

// Note: handle, list, and struct elements are referenced before this call is made

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
        case MincStructType:
            _u.mstruct = rhs._u.mstruct;
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
            TPRINT("%p\n", _u.handle);
            break;
        case MincListType:
            TPRINT("%p\n", _u.list);
            break;
        case MincStringType:
            TPRINT("%s\n", _u.string);
            break;
        case MincStructType:
            TPRINT("%p\n", _u.mstruct);
            break;
        case MincVoidType:
            TPRINT("void\n");
            break;
    }
}

// Public operators

const MincValue& MincValue::operator = (const MincValue &rhs)
{
    ENTER();
    if (rhs.type == MincHandleType)
        ref_handle(rhs._u.handle);
    else if (rhs.type == MincListType)
        RefCounted::ref(rhs._u.list);
    else if (rhs.type == MincStructType)
        RefCounted::ref(rhs._u.mstruct);
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

bool MincValue::operator == (const MincValue &rhs)
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

bool MincValue::operator != (const MincValue &rhs)
{
    return !(*this == rhs);
}

bool MincValue::operator < (const MincValue &rhs)
{
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("attempt to compare variables having different types"));
    ThrowIf(this->type != MincFloatType, InvalidTypeException("can't compare this type of object"));
    return cmp(_u.number, rhs._u.number) == -1;
}

bool MincValue::operator > (const MincValue &rhs)
{
    ThrowIf(rhs.type != this->type, NonmatchingTypeException("attempt to compare variables having different types"));
    ThrowIf(this->type != MincFloatType, InvalidTypeException("can't compare this type of object"));
    return cmp(_u.number, rhs._u.number) == 1;
}

bool MincValue::operator <= (const MincValue &rhs)
{
    return !(*this > rhs);
}

bool MincValue::operator >= (const MincValue &rhs)
{
    return !(*this < rhs);
}
