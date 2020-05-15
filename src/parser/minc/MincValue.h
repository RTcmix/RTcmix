//
//  MincValue.h
//  RTcmix
//
//  Created by Douglas Scott on 12/30/19.
//

#ifndef MincValue_h
#define MincValue_h

#include "minc_internal.h"
#include <stddef.h>

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

// A MincStruct contains a Symbol pointer which points to a linked list of Symbols
// which represent the elements of a particular MinC-defined struct.

class Symbol;

class MincStruct : public MincObject, public RefCounted
{
public:
    MincStruct() : _memberList(NULL) {}
    ~MincStruct();
    Symbol *    addMember(const char *name, MincDataType type, int scope);
    Symbol *    lookupMember(const char *name);
    Symbol *    members() { return _memberList; }
    void        print();
protected:
    Symbol *    _memberList;
};

class MincValue {
public:
    MincValue() : type(MincVoidType) { _u.list = NULL; }
    MincValue(MincFloat f) : type(MincFloatType) { _u.number = f; }
    MincValue(MincString s) : type(MincStringType) { _u.string = s; }
    MincValue(MincHandle h);
    MincValue(MincList *l);
    MincValue(MincStruct *str);
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
    
    const MincValue& operator[] (const MincValue &index) const;    // for MincList access
    MincValue& operator[] (const MincValue &index);    // for MincList access
    
    operator MincFloat() const { return _u.number; }
    operator MincString() const { return _u.string; }
    operator MincHandle() const { return _u.handle; }
    operator MincList *() const { return _u.list; }
    operator MincStruct *() const { return _u.mstruct; }
    operator bool() const { return (type == MincFloatType) ? _u.number != 0.0 : _u.string != NULL; }
    
    bool operator == (const MincValue &rhs);
    bool operator != (const MincValue &rhs);
    bool operator < (const MincValue &rhs);
    bool operator > (const MincValue &rhs);
    bool operator <= (const MincValue &rhs);
    bool operator >= (const MincValue &rhs);
    
    MincDataType    dataType() const { return type; }
    void zero() { _u.list = NULL; }        // zeroes without changing type
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
        MincStruct *mstruct;
    } _u;
};

#endif /* MincValue_h */
