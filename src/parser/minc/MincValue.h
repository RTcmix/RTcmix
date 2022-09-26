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
#include <map>

/* A MincList contains an array of MincValues, whose underlying data
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

/* A MincMap is a set of MincValues, like a MincList, except they can be
   set and retrieved via MincValue "keys".  Like lists, they support mixed
   types.
 */
class MincMap : public MincObject, public RefCounted
{
private:
    struct MincValueCmp {
        bool operator()(const MincValue& lhs, const MincValue& rhs) const;
    };
public:
    MincMap();
    int len() const { return (int) map.size(); }
    bool contains(const MincValue &element);
    std::map<MincValue, MincValue, MincValueCmp> map;
    void        print();
protected:
    virtual ~MincMap();
};

// A MincStruct contains a Symbol pointer which points to a linked list of Symbols
// which represent the elements of a particular MinC-defined struct.

class Symbol;

class MincStruct : public MincObject, public RefCounted
{
public:
    MincStruct(const char *typeName) : _typeName(typeName), _memberList(NULL) {}
    const char *typeName() const { return _typeName; }
    Symbol *    addMember(const char *name, const MincValue &value, int scope, const char *structTypeName);
    Symbol *    lookupMember(const char *name);
    Symbol *    members() { return _memberList; }
    void        print();
protected:
    virtual ~MincStruct();
protected:
    const char *    _typeName;
    Symbol *        _memberList;
};

// A MincFunction contains two Nodes which contain the argument list
// and the list of operations to be carried out by the function body.

class Node;


class MincFunction : public MincObject, public RefCounted {
public:
    enum Type {
        Standalone  = 0x10,
        Method      = 0x20
    };
    MincFunction(Node *argumentList, Node *functionBody, MincFunction::Type type=MincFunction::Standalone);
    void    handleThis(std::vector<Symbol *> &symbolStack);
    void    copyArguments();
    Node *  execute();
protected:
    virtual ~MincFunction();
private:
    Node *  _argumentList;
    Node *  _functionBody;
    MincFunction::Type _type;
};

class MincValue {
public:
    MincValue() : type(MincVoidType) { _u.list = NULL; }
    MincValue(MincFloat f) : type(MincFloatType) { _u.number = f; }
    MincValue(MincString s) : type(MincStringType) { _u.string = s; }
    MincValue(MincHandle h);
    MincValue(MincList *l);
    MincValue(MincMap *m);
    MincValue(MincStruct *str);
    MincValue(MincFunction *func);
    MincValue(MincDataType type);
    MincValue(const MincValue &rhs);
    ~MincValue();
    const MincValue& operator = (const MincValue &rhs);
    const MincValue& operator = (MincFloat f);
    const MincValue& operator = (MincString s);
    const MincValue& operator = (MincHandle h);
    const MincValue& operator = (MincList *l);
    const MincValue& operator = (MincMap *m);
    const MincValue& operator = (MincFunction *f);

    const MincValue& operator += (const MincValue &rhs);
    const MincValue& operator -= (const MincValue &rhs);
    const MincValue& operator *= (const MincValue &rhs);
    const MincValue& operator /= (const MincValue &rhs);
    
    const MincValue& operator[] (const MincValue &index) const;    // for MincList,MincMap access
    MincValue& operator[] (const MincValue &index);                 // for MincList, MincMap access
    
    operator MincFloat() const { return _u.number; }
    operator MincString() const { return _u.string; }
    operator MincHandle() const { return _u.handle; }
    operator MincList *() const { return _u.list; }
    operator MincMap *() const { return _u.map; }
    operator MincStruct *() const { return _u.mstruct; }
    operator MincFunction *() const { return _u.mfunc; }
    operator bool() const { return (type == MincFloatType) ? _u.number != 0.0 : _u.string != NULL; }
    
    unsigned long long rawValue() const { return _u.raw; }
    
    bool operator == (const MincValue &rhs) const;
    bool operator != (const MincValue &rhs) const;
    bool operator < (const MincValue &rhs) const;
    bool operator > (const MincValue &rhs) const;
    bool operator <= (const MincValue &rhs) const;
    bool operator >= (const MincValue &rhs) const;
    
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
        MincMap *map;
        MincStruct *mstruct;
        MincFunction *mfunc;
        unsigned long long raw;     // used for raw comparison
    } _u;
};

#endif /* MincValue_h */
