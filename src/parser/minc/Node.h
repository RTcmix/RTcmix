//
//  Node.h
//  RTcmixTest
//
//  Created by Douglas Scott on 10/3/16.
//
//

#ifndef RT_NODE_H
#define RT_NODE_H

#include "minc_internal.h"
#include "MincValue.h"
#include "RefCounted.h"

// NODE_DEBUG enables logging of Node creation and destruction
#undef NODE_DEBUG

#ifdef NODE_DEBUG
static char sBuf[256];
#define NPRINT(...) do { snprintf(sBuf, 256, __VA_ARGS__); rtcmix_print("%s", sBuf); } while(0)
#else
#define NPRINT(...)
#endif

/* intermediate tree representation */

typedef enum {
	eNodeZero = 0,
	eNodeSeq,
	eNodeStore,
	eNodeList,
	eNodeListElem,
	eNodeEmptyListElem,
	eNodeSubscriptRead,
	eNodeSubscriptWrite,
    eNodeSubscriptIncrement,
    eNodeMember,
	eNodeOpAssign,
	eNodeLoadSym,
	eNodeAutoDeclLoadSym,
	eNodeConstf,
	eNodeString,
    eNodeMemberDecl,
    eNodeStructDef,
	eNodeFuncDef,
    eNodeMethodDef,
	eNodeArgList,
	eNodeArgListElem,
	eNodeRet,
	eNodeFuncBodySeq,
	eNodeFuncCall,
    eNodeMethodCall,
	eNodeAnd,
	eNodeOr,
	eNodeOperator,
	eNodeUnaryOperator,
	eNodeNot,
	eNodeRelation,
	eNodeIf,
	eNodeWhile,
	eNodeFor,
	eNodeIfElse,
	eNodeDecl,
    eNodeStructDecl,
	eNodeFuncDecl,
    eNodeMethodDecl,
	eNodeBlock,
	eNodeNoop
} NodeKind;

class Node : public MincObject, public RefCounted
{
//protected:					TODO: FINISH FULL CLASS
public:
	NodeKind        kind;
	MincDataType    _type;
	OpKind          op;
	MincValue 		v;
	int				lineno;         /* used for error statements */
    const char *    includeFilename;   /* used for error statements */
public:
	Node(OpKind op, NodeKind kind);
	const char *		classname() const;
    const char *		name() const;
	MincDataType		dataType() const { return v.dataType(); }
	virtual Node*		child(int index) const { return NULL; }
	void				setSymbol(Symbol *sym) { u.symbol = sym; }
	Symbol *			symbol() const { return u.symbol; }
    void                setValue(const MincValue &value) { v = value; }
	const MincValue&	value() const { return v; }
	Node*				exct();

    Node *              copyValue(Node *, bool allowTypeOverwrite=true, bool suppressOverwriteWarning=false);
    Node *              copyValue(Symbol *, bool allowTypeOverwrite=true, bool suppressOverwriteWarning=false);
	void				print();
protected:
    virtual             ~Node();
	virtual Node*		doExct() = 0;
protected:
	union {
		Symbol *symbol;
		double number;
		const char *string;
	} u;
};

class NodeNoop : public Node
{
public:
	NodeNoop() : Node(OpFree, eNodeNoop) { }
protected:
    virtual                ~NodeNoop();
	virtual Node*		doExct() { return this; }
};

class Node1Child : public Node
{
	Node* _child;
public:
    Node1Child(OpKind op, NodeKind kind, Node *n1) : Node(op, kind), _child(n1) { RefCounted::ref(n1); }
	virtual Node*		child(int index) const { return (index == 0) ? _child : NULL; }
protected:
    virtual        ~Node1Child() { RefCounted::unref(_child); }
};

class Node2Children : public Node
{
	Node* _children[2];
public:
	Node2Children(OpKind op, NodeKind kind, Node *n1, Node *n2)
        : Node(op, kind) { _children[0] = n1; _children[1]= n2; n1->ref(); n2->ref(); }
	virtual Node*	child(int index) const { return (index < 2) ? _children[index] : NULL; }
protected:
    virtual            ~Node2Children() { _children[0]->unref(); _children[1]->unref(); }
};

class Node3Children : public Node
{
	Node* _children[3];
public:
	Node3Children(OpKind op, NodeKind kind, Node *n1, Node *n2, Node *n3)
    : Node(op, kind) {
        _children[0] = n1; _children[1]= n2; _children[2] = n3;
        n1->ref(); n2->ref(); n3->ref();
    }
protected:
	virtual			~Node3Children() {
		_children[0]->unref(); _children[1]->unref(); _children[2]->unref();
	}
	virtual Node*		child(int index) const { return (index < 3) ? _children[index] : NULL; }
};

class NodeSeq : public Node2Children
{
public:
	NodeSeq(Node *n1, Node *n2) : Node2Children(OpFree, eNodeSeq, n1, n2) {
		NPRINT("NodeSeq(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeOp : public Node2Children
{
public:
	NodeOp(OpKind op, Node *n1, Node *n2) : Node2Children(op, eNodeOperator, n1, n2) {
		NPRINT("NodeOperator(%d, %p, %p) => %p\n", op, n1, n2, this);
	}
protected:
	virtual Node*		doExct();
private:
	Node* do_op_string(const char *str1, const char *str2, OpKind op);
	Node* do_op_num(const MincFloat val1, const MincFloat val2, OpKind op);
	Node* do_op_handle_num(const MincHandle val1, const MincFloat val2, OpKind op);
	Node* do_op_num_handle(const MincFloat val1, const MincHandle val2, OpKind op);
	Node* do_op_handle_handle(const MincHandle val1, const MincHandle val2, OpKind op);
	Node* do_op_list_float(const MincList *srcList, const MincFloat val, const OpKind op);
	Node* do_op_list_list(const MincList *list1, const MincList *list2, const OpKind op);
    Node* do_op_float_list(const MincFloat val, const MincList *srcList, const OpKind op);
};

class NodeUnaryOperator : public Node1Child
{
public:
	NodeUnaryOperator(OpKind op, Node *n1) : Node1Child(op, eNodeUnaryOperator, n1) {
		NPRINT("NodeUnaryOperator(%d, %p) => %p\n", op, n1, this);
	}
protected:
	virtual Node*		doExct();
};

/* store a value into a variable */
class NodeStore : public Node2Children
{
public:
	NodeStore(Node *n1, Node *n2, bool allowTypeOverwrite=true)
        : Node2Children(OpFree, eNodeStore, n1, n2), _allowTypeOverwrite(allowTypeOverwrite) {
		NPRINT("NodeStore (%p, %p, %d) => %p\n", n1, n2, allowTypeOverwrite, this);
	}
protected:
	virtual Node*		doExct();
private:
    bool    _allowTypeOverwrite;        // true for everything except struct members
};

/* like NodeStore, but modify value before storing into variable */
class NodeOpAssign : public Node2Children
{
public:
	NodeOpAssign(Node *n1, Node *n2, OpKind op) : Node2Children(op, eNodeOpAssign, n1, n2) {
		NPRINT("NodeOpAssign(%p, %p, op=%d) => %p\n", n1, n2, op, this);
	}
protected:
	virtual Node*		doExct();
};

/* looks up symbol name and get the symbol.  Converts symbol table entry into Node
	or initialize Node to a symbol entry.  This symbol can be an object or a function.
 */
class NodeLoadSym : public Node
{
public:
	NodeLoadSym(const char *symbolName) : Node(OpFree, eNodeLoadSym), _symbolName(symbolName) {
		NPRINT("NodeLoadSym('%s') => %p\n", symbolName, this);
	}
protected:
	NodeLoadSym(const char *symbolName, NodeKind kind) : Node(OpFree, kind), _symbolName(symbolName) {}
	virtual Node*		doExct();
	virtual Node *      finishExct();
	const char *		symbolName() const { return _symbolName; }
private:
    const char *_symbolName;       /* used for function name, symbol name (for lookup) */
};

/* looks up symbol name and get the symbol, and auto-declares it if not found
 converts symbol table entry into tree or initialize tree node to a symbol entry
 */
class NodeAutoDeclLoadSym : public NodeLoadSym
{
public:
	NodeAutoDeclLoadSym(const char *symbolName) : NodeLoadSym(symbolName, eNodeAutoDeclLoadSym) {
		NPRINT("NodeAutoDeclLoadSym('%s') => %p\n", symbolName, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeString : public Node
{
public:
	NodeString(const char *str) : Node(OpFree, eNodeString) {
		u.string = str;
		NPRINT("NodeString('%s') => %p\n", str, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeConstf : public Node
{
public:
	NodeConstf(MincFloat num) : Node(OpFree, eNodeConstf) {
		u.number = num;
		NPRINT("NodeConstf(%f) => %p\n", num, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeArgListElem : public Node2Children
{
public:
	NodeArgListElem(Node *n1, Node *n2) : Node2Children(OpFree, eNodeArgListElem, n1, n2) {
		NPRINT("NodeArgListElem(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeArgList : public Node1Child
{
public:
	NodeArgList(Node *n1) : Node1Child(OpFree, eNodeArgList, n1) {
		NPRINT("NodeArgList(%p) => %p\n", n1, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeRet : public Node1Child
{
public:
	NodeRet(Node *n1) : Node1Child(OpFree, eNodeRet, n1) {
		NPRINT("NodeRet(%p) => %p\n", n1, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeMemberDecl : public Node
{
public:
    NodeMemberDecl(const char *name, MincDataType type, const char *subtype=NULL) : Node(OpFree, eNodeMemberDecl), _symbolName(name), _symbolSubtype(subtype) {
        this->_type = type;
        if (_symbolSubtype == NULL) {
            NPRINT("NodeMemberDecl('%s', %s) => %p\n", name, MincTypeName(type), this);
        }
        else {
            NPRINT("NodeMemberDecl('%s', %s %s) => %p\n", name, MincTypeName(type), _symbolSubtype, this);
        }
    }
protected:
    virtual Node*        doExct();
private:
    const char *    _symbolName;
    const char *    _symbolSubtype;
};

// Struct definition node.  Stores "template" for a just-declared struct.
//  n1 NodeSeq of NodeDecls for elements

class NodeStructDef : public Node1Child
{
public:
    NodeStructDef(const char *name, Node *n1) : Node1Child(OpFree, eNodeStructDef, n1), _typeName(name) {
        NPRINT("NodeStructDef(%s, %p) => %p\n", name, n1, this);
    }
protected:
    virtual Node*        doExct();
private:
    const char *    _typeName;
};

class NodeFuncBodySeq : public Node2Children
{
public:
	NodeFuncBodySeq(Node *n1, Node *n2) : Node2Children(OpFree, eNodeFuncBodySeq, n1, n2) {
		NPRINT("NodeFuncBodySeq(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

// Function definition node
//	funcDecl Lookup node
//	argList  NodeArgList (argument symbol decls)
//	funcBody NodeFuncBodySeq function body (statements), which returns value

class NodeFuncDef : public Node3Children
{
public:
	NodeFuncDef(Node *funcDecl, Node *argList, Node *funcBody) : Node3Children(OpFree, eNodeFuncDef, funcDecl, argList, funcBody), _isMethod(false) {
		NPRINT("NodeFuncDef(%p, %p, %p) => %p\n", funcDecl, argList, funcBody, this);
	}
protected:
    NodeFuncDef(Node *funcDecl, Node *argList, Node *funcBody, NodeKind kind) : Node3Children(OpFree, kind, funcDecl, argList, funcBody), _isMethod(kind==eNodeMethodDef) {}
	virtual Node*		doExct();
    bool                _isMethod;
};

// Method definition node
//    funcDecl Lookup node
//    argList  NodeArgList (argument symbol decls)
//    funcBody NodeFuncBodySeq function body (statements), which returns value

class NodeMethodDef : public NodeFuncDef
{
public:
    NodeMethodDef(Node *funcDecl, Node *argList, Node *funcBody) : NodeFuncDef(funcDecl, argList, funcBody, eNodeMethodDef) {
        NPRINT("NodeMethodDef(%p, %p, %p) => %p\n", funcDecl, argList, funcBody, this);
    }
};

// Private base class to allow sharing of functionality

class MincFunctionHandler
{
public:
    MincFunctionHandler() {}
    Node *                callMincFunction(MincFunction *function, const char *functionName, Symbol *thisSymbol=NULL);
};

//  Function call node
//  n1 Function symbol node
//  n2 Function arguments list

class NodeFunctionCall : public Node2Children, private MincFunctionHandler
{
public:
    NodeFunctionCall(Node *func, Node *args) : Node2Children(OpFree, eNodeFuncCall, func, args) {
		NPRINT("NodeFunctionCall(%p, %p) => %p\n", func, args, this);
	}
protected:
	virtual Node*		doExct();
private:
    bool                callConstructor(const char *functionName);
    void                callBuiltinFunction(const char *functionName);
};

//  Method call node
//  n1 Object for method
//  s  Method name
//  n2 Function arguments list

class NodeMethodCall : public Node2Children, private MincFunctionHandler
{
public:
    NodeMethodCall(Node *obj, const char *methodName, Node *args) : Node2Children(OpFree, eNodeMethodCall, obj, args), _methodName(methodName) {
        NPRINT("NodeMethodCall(%p, '%s', %p) => %p\n", obj, methodName, args, this);
    }
protected:
    virtual Node *  doExct();
private:
    bool            callObjectMethod(Symbol *thisSymbol, const char *methodName);
private:
    const char *    _methodName;
};

class NodeAnd : public Node2Children
{
public:
	NodeAnd(Node *lhs, Node *rhs) : Node2Children(OpFree, eNodeAnd, lhs, rhs) {
		NPRINT("NodeAnd(%p, %p) => %p\n", lhs, rhs, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeOr : public Node2Children
{
public:
	NodeOr(Node *lhs, Node *rhs) : Node2Children(OpFree, eNodeOr, lhs, rhs) {
		NPRINT("NodeOr(%p, %p) => %p\n", lhs, rhs, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeNot : public Node1Child
{
public:
	NodeNot(Node *n1) : Node1Child(OpFree, eNodeNot, n1) {
		NPRINT("NodeNot(%p) => %p\n", n1, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeRelation : public Node2Children
{
public:
	NodeRelation(OpKind op, Node *n1, Node *n2) : Node2Children(op, eNodeRelation, n1, n2) {
		NPRINT("NodeRelation(%d, %p, %p) => %p\n", op, n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

/* Create list: either an argument list or a user array.  Why do we
 not separate these two things?  Because at the time when we need
 to push the list elements onto a stack, we don't know whether they
 form part of a user list or an argument list.
 */
class NodeList : public Node1Child
{
public:
	// n1 == tail of a NodeListElem linked list
	NodeList(Node *n1) : Node1Child(OpFree, eNodeList, n1) {
		NPRINT("NodeList(%p) => %p\n", n1, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeListElem : public Node2Children
{
public:
	NodeListElem(Node *elem, Node *payload) : Node2Children(OpFree, eNodeListElem, elem, payload) {
		NPRINT("NodeListElem(%p, %p) => %p\n", elem, payload, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeEmptyListElem : public Node
{
public:
	NodeEmptyListElem() : Node(OpFree, eNodeEmptyListElem) {
		NPRINT("NodeEmptyListElem() => %p\n", this);
	}
protected:
	virtual Node*		doExct() { return this; }
};

class Subscript
{
public:
    MincValue readValueAtIndex(Node *listNode, Node *indexNode);
    void writeValueToIndex(Node *listNode, Node *indexNode, const MincValue &value);
};

class NodeSubscriptRead : public Node2Children, private Subscript
{
public:
	NodeSubscriptRead(Node *n1, Node *n2) : Node2Children(OpFree, eNodeSubscriptRead, n1, n2) {
		NPRINT("NodeSubscriptRead(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
    void                readAtSubscript();
    void                searchWithMapKey();
};

class NodeSubscriptWrite : public Node3Children, private Subscript
{
public:
	NodeSubscriptWrite(Node *n1, Node *n2, Node *n3) : Node3Children(OpFree, eNodeSubscriptWrite, n1, n2, n3) {
		NPRINT("NodeSubscriptWrite(%p, %p, %p) => %p\n", n1, n2, n3, this);
	}
protected:
	virtual Node*		doExct();
    void                writeToSubscript(Node *listNode, Node *indexNode, const MincValue &value);
    void                writeWithMapKey(Node *mapNode, Node *indexNode, const MincValue &value);
};

class NodeSubscriptIncrement : public Node3Children, private Subscript
{
public:
    NodeSubscriptIncrement(Node *n1, Node *n2, Node *n3) : Node3Children(OpFree, eNodeSubscriptIncrement, n1, n2, n3) {
        NPRINT("NodeSubscriptIncrement(%p, %p, %p) => %p\n", n1, n2, n3, this);
    }
protected:
    virtual Node*		doExct();
    void                incrementSubscript(Node *listNode, Node *indexNode, Node *valueNode);
};

// NodeMemberAccess returns symbol for member var or method function from RHS of dot operator

class NodeMemberAccess : public Node1Child
{
public:
    NodeMemberAccess(Node *n1, const char *memberName) : Node1Child(OpFree, eNodeMember, n1), _memberName(memberName) {
        NPRINT("NodeMemberAccess(%p, '%s') => %p\n", n1, memberName, this);
    }
protected:
    virtual Node*        doExct();
private:
    const char *_memberName;
};

class NodeIf : public Node2Children
{
public:
	NodeIf(Node *n1, Node *n2) : Node2Children(OpFree, eNodeIf, n1, n2) {
		NPRINT("NodeIf(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeIfElse : public Node3Children
{
public:
	NodeIfElse(Node *n1, Node *n2, Node *n3) : Node3Children(OpFree, eNodeIfElse, n1, n2, n3) {
		NPRINT("NodeIfElse(%p, %p, %p) => %p\n", n1, n2, n3, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeFor : public Node3Children
{
	Node *_child4;
public:
	NodeFor(Node *n1, Node *n2, Node *n3, Node *n4) : Node3Children(OpFree, eNodeFor, n1, n2, n3), _child4(n4) {
		NPRINT("NodeFor(%p, %p, %p, <e4>) => %p\n", n1, n2, n3, this);
        n4->ref();
	}
	virtual ~NodeFor() { _child4->unref(); }
protected:
	virtual Node*		doExct();
};

class NodeWhile : public Node2Children
{
public:
	NodeWhile(Node *n1, Node *n2) : Node2Children(OpFree, eNodeWhile, n1, n2) {
		NPRINT("NodeWhile(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeDecl : public Node
{
public:
	NodeDecl(const char *name, MincDataType type) : Node(OpFree, eNodeDecl), _symbolName(name) {
		this->_type = type;		// TODO
		NPRINT("NodeDecl('%s') => %p\n", name, this);
	}
protected:
	virtual Node*		doExct();
private:
	const char *	_symbolName;
};

class NodeStructDecl : public Node1Child
{
public:
    NodeStructDecl(const char *name, const char *typeName, Node *initializerList=NULL) : Node1Child(OpFree, eNodeStructDecl, initializerList),
        _symbolName(name), _typeName(typeName) {
        this->_type = MincStructType;
        NPRINT("NodeStructDecl('struct %s %s') => %p\n", _typeName, _symbolName, this);
    }
protected:
    virtual Node*        doExct();
private:
    const char *    _symbolName;
    const char *    _typeName;
};

class NodeFuncDecl : public Node
{
public:
	NodeFuncDecl(const char *name, MincDataType type) : Node(OpFree, eNodeFuncDecl), _symbolName(name) {
		this->_type = type;		// TODO
		NPRINT("NodeFuncDecl('%s') => %p\n", name, this);
	}
protected:
	virtual Node*		doExct();
private:
	const char *	_symbolName;
};

class NodeMethodDecl : public Node
{
public:
    NodeMethodDecl(const char *name, const char *structTypeName, MincDataType type)
        : Node(OpFree, eNodeMethodDecl),
          _symbolName(name), _structTypeName(structTypeName) {
        this->_type = type;        // TODO
        NPRINT("NodeMethodDecl('%s', '%s') => %p\n", name, structTypeName, this);
    }
protected:
    virtual Node*        doExct();
private:
    const char *    _symbolName;
    const char *    _structTypeName;
};

class NodeBlock : public Node1Child
{
public:
	NodeBlock(Node *n1) : Node1Child(OpFree, eNodeBlock, n1) {
		NPRINT("NodeBlock(%p) => %p\n", n1, this);
	}
protected:
	virtual Node*		doExct();
};

#endif /* defined(RT_NODE_H) */
