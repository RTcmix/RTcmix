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
	eNodeSeq = 1,
	eNodeStore,
	eNodeList,
	eNodeListElem,
	eNodeEmptyListElem,
	eNodeSubscriptRead,
	eNodeSubscriptWrite,
    eNodeSubscriptOpAssign,
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
    eNodeTernary,
	eNodeDecl,
    eNodeStructDecl,
	eNodeFuncDecl,
    eNodeMethodDecl,
	eNodeBlock,
	eNodeNoop,
	eNodeSwitch,
	eNodeCaseClauseList,
	eNodeCaseClause,
	eNodeCaseLabelList,
	eNodeCaseLabel,
	eNodeDefaultClause
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
	void				setSymbol(Symbol *sym) { _symbol = sym; }
	Symbol *			symbol() const { return _symbol; }
    void                setValue(const MincValue &value) { v = value; }
	const MincValue&	value() const { return v; }
	Node*				exct();

    Node *              copyValue(Node *, bool allowTypeOverwrite=true, bool suppressOverwriteWarning=false);
    Node *              copyValue(Symbol *, bool allowTypeOverwrite=true, bool suppressOverwriteWarning=false);
	void				print();
protected:
    virtual             ~Node();
	virtual Node*		doExct() = 0;
	void				copyValue(const MincValue &value, bool allowTypeOverwrite=true, bool suppressOverwriteWarning=false);
    void                printValue();
protected:
    Symbol *            _symbol;
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

// Base for nodes whose child count is known only at construction

template <int ChildCount>
class NodeWithChildren : public Node {
	Node* _children[ChildCount];
public:
	NodeWithChildren(OpKind op, NodeKind kind, Node *n1, Node *n2)
		: Node(op, kind) { init(); setChild(0, n1); setChild(1, n2); }
	NodeWithChildren(OpKind op, NodeKind kind, Node *n1, Node *n2, Node *n3)
	: Node(op, kind) { init(); setChild(0, n1); setChild(1, n2); setChild(2, n3); }
	virtual Node*	child(int index) const { return (index < ChildCount) ? _children[index] : NULL; }
protected:
	void	init() { for (int i = 0; i < ChildCount; ++i) { _children[i] = NULL; } }
	void	setChild(int index, Node *n) {		// stores n in slot 'index' and refs it
		_children[index] = n;
		RefCounted::ref(n);
	}
	virtual	~NodeWithChildren() {
		for (int i = 0; i < ChildCount; ++i) { RefCounted::unref(_children[i]); }
	}
};

class NodeSeq : public NodeWithChildren<2>
{
public:
	NodeSeq(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeSeq, n1, n2) {
		NPRINT("NodeSeq(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class OperationBase
{
public:
    OperationBase();
    Node *doOperation(Node *node, const MincValue &lhs, const MincValue &rhs, OpKind op);
private:
    Node* do_op_string(Node *node, const char *str1, const char *str2, OpKind op);
    Node* do_op_num(Node *node, MincFloat val1, MincFloat val2, OpKind op);
    Node* do_op_handle_num(Node *node, MincHandle  val1, MincFloat val2, OpKind op);
    Node* do_op_num_handle(Node *node, MincFloat val1, MincHandle  val2, OpKind op);
    Node* do_op_handle_handle(Node *node, MincHandle  val1, MincHandle  val2, OpKind op);
    Node* do_op_list_float(Node *node, const MincList *srcList, MincFloat val, OpKind  op);
    Node* do_op_list_list(Node *node, const MincList *list1, const MincList *list2, OpKind  op);
    Node* do_op_float_list(Node *node, MincFloat val, const MincList *srcList, OpKind  op);
	// The struct operators do not pass the 'val' argument because it is processed into an argument list.
	template <class T>
	Node *do_op_struct(Node *node, MincStruct *srcStruct, OpKind op);
};

class NodeOp : public NodeWithChildren<2>, private OperationBase
{
public:
	NodeOp(OpKind op, Node *n1, Node *n2) : NodeWithChildren<2>(op, eNodeOperator, n1, n2) {
		NPRINT("NodeOperator(%d, %p, %p) => %p\n", op, n1, n2, this);
	}
protected:
	virtual Node*		doExct();
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
class NodeStore : public NodeWithChildren<2>
{
public:
	NodeStore(Node *n1, Node *n2, bool allowTypeOverwrite=true)
        : NodeWithChildren<2>(OpFree, eNodeStore, n1, n2), _allowTypeOverwrite(allowTypeOverwrite) {
		NPRINT("NodeStore (%p, %p, %d) => %p\n", n1, n2, allowTypeOverwrite, this);
	}
protected:
	virtual Node*		doExct();
private:
    bool    _allowTypeOverwrite;        // true for everything except struct members
};

/* like NodeStore, but modify value before storing into variable */
class NodeOpAssign : public NodeWithChildren<2>, private OperationBase
{
public:
	NodeOpAssign(Node *n1, Node *n2, OpKind op) : NodeWithChildren<2>(op, eNodeOpAssign, n1, n2) {
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
	NodeString(const char *str) : Node(OpFree, eNodeString), string(str) {
		NPRINT("NodeString('%s') => %p\n", string, this);
	}
protected:
	virtual Node*		doExct();
private:
    const char *string;
};

class NodeConstf : public Node
{
public:
	NodeConstf(MincFloat num) : Node(OpFree, eNodeConstf), number(num) {
		NPRINT("NodeConstf(%f) => %p\n", number, this);
	}
protected:
	virtual Node*		doExct();
private:
    double number;
};

class NodeArgListElem : public NodeWithChildren<2>
{
public:
	NodeArgListElem(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeArgListElem, n1, n2) {
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
//  basename name of struct from which this is derived (optional)

class NodeStructDef : public Node1Child
{
public:
    NodeStructDef(const char *name, Node *n1, const char *basename=NULL) : Node1Child(OpFree, eNodeStructDef, n1), _typeName(name), _baseName(basename) {
        NPRINT("NodeStructDef(%s, %p) => %p\n", name, n1, this);
    }
protected:
    virtual Node*        doExct();
private:
    const char *    _typeName;
    const char *    _baseName;
};

class NodeFuncBodySeq : public NodeWithChildren<2>
{
public:
	NodeFuncBodySeq(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeFuncBodySeq, n1, n2) {
		NPRINT("NodeFuncBodySeq(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

// Function definition node
//	funcDecl Lookup node
//	argList  NodeArgList (argument symbol decls)
//	funcBody NodeFuncBodySeq function body (statements), which returns value

class NodeFuncDef : public NodeWithChildren<3>
{
public:
	NodeFuncDef(Node *funcDecl, Node *argList, Node *funcBody) : NodeWithChildren<3>(OpFree, eNodeFuncDef, funcDecl, argList, funcBody), _isMethod(false) {
		NPRINT("NodeFuncDef(%p, %p, %p) => %p\n", funcDecl, argList, funcBody, this);
	}
protected:
    NodeFuncDef(Node *funcDecl, Node *argList, Node *funcBody, NodeKind kind) : NodeWithChildren<3>(OpFree, kind, funcDecl, argList, funcBody), _isMethod(kind==eNodeMethodDef) {}
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
    static MincValue	callMincFunction(MincFunction *function, const char *functionName, MincStruct *thisStruct=NULL);
};

//  Function call node
//  n1 Function symbol node
//  n2 Function arguments list

class NodeFunctionCall : public NodeWithChildren<2>, private MincFunctionHandler
{
public:
    NodeFunctionCall(Node *func, Node *args) : NodeWithChildren<2>(OpFree, eNodeFuncCall, func, args) {
		NPRINT("NodeFunctionCall(%p, %p) => %p\n", func, args, this);
	}
protected:
	virtual Node*		doExct();
private:
    bool                callConstructor(const char *functionName);
    void                callBuiltinFunction(const char *functionName);
    void                callInitMethodIfPresent(MincStruct *theStruct);
};

//  Method call node
//  n1 Object for method
//  s  Method name
//  n2 Function arguments list

class NodeMethodCall : public NodeWithChildren<2>, private MincFunctionHandler
{
public:
    NodeMethodCall(Node *obj, const char *methodName, Node *args) : NodeWithChildren<2>(OpFree, eNodeMethodCall, obj, args), _methodName(methodName) {
        NPRINT("NodeMethodCall(%p, '%s', %p) => %p\n", obj, methodName, args, this);
    }
protected:
    virtual Node *  doExct();
private:
    bool            callObjectMethod(MincValue objectValue, const char *methodName);
private:
    const char *    _methodName;
};

class NodeAnd : public NodeWithChildren<2>
{
public:
	NodeAnd(Node *lhs, Node *rhs) : NodeWithChildren<2>(OpFree, eNodeAnd, lhs, rhs) {
		NPRINT("NodeAnd(%p, %p) => %p\n", lhs, rhs, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeOr : public NodeWithChildren<2>
{
public:
	NodeOr(Node *lhs, Node *rhs) : NodeWithChildren<2>(OpFree, eNodeOr, lhs, rhs) {
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

class NodeRelation : public NodeWithChildren<2>
{
public:
	NodeRelation(OpKind op, Node *n1, Node *n2) : NodeWithChildren<2>(op, eNodeRelation, n1, n2) {
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

class NodeListElem : public NodeWithChildren<2>
{
public:
	NodeListElem(Node *elem, Node *payload) : NodeWithChildren<2>(OpFree, eNodeListElem, elem, payload) {
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

// Subscript class owns implementtion for read/write access to lists and maps

class Subscript
{
public:
    MincValue readValueAtIndex(Node *listNode, Node *indexNode);
    void writeValueToIndex(Node *listNode, Node *indexNode, const MincValue &value);
    MincValue searchWithMapKey(Node *mapNode, Node *key);
    void writeWithMapKey(Node *mapNode, Node *keyNode, const MincValue &value);
};

class NodeSubscriptRead : public NodeWithChildren<2>, private Subscript
{
public:
	NodeSubscriptRead(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeSubscriptRead, n1, n2) {
		NPRINT("NodeSubscriptRead(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeSubscriptWrite : public NodeWithChildren<3>, private Subscript
{
public:
	NodeSubscriptWrite(Node *n1, Node *n2, Node *n3) : NodeWithChildren<3>(OpFree, eNodeSubscriptWrite, n1, n2, n3) {
		NPRINT("NodeSubscriptWrite(%p, %p, %p) => %p\n", n1, n2, n3, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeSubscriptOpAssign : public NodeWithChildren<3>, private Subscript, private OperationBase
{
public:
    NodeSubscriptOpAssign(Node *n1, Node *n2, Node *n3, OpKind op) : NodeWithChildren<3>(op, eNodeSubscriptOpAssign, n1, n2, n3) {
        NPRINT("NodeSubscriptOpAssign(%p, %p, %p, op=%d) => %p\n", n1, n2, n3, op, this);
    }
protected:
    virtual Node*		doExct();
    void                operateOnSubscript(Node *listNode, Node *indexNode, Node *valueNode, OpKind op);
    void                operateOnMapLookup(Node *mapNode, Node *keyNode, Node *valueNode, OpKind op);
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

class NodeIf : public NodeWithChildren<2>
{
public:
	NodeIf(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeIf, n1, n2) {
		NPRINT("NodeIf(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeIfElse : public NodeWithChildren<3>
{
public:
	NodeIfElse(Node *n1, Node *n2, Node *n3) : NodeWithChildren<3>(OpFree, eNodeIfElse, n1, n2, n3) {
		NPRINT("NodeIfElse(%p, %p, %p) => %p\n", n1, n2, n3, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeFor : public NodeWithChildren<3>
{
	Node *_child4;
public:
	NodeFor(Node *n1, Node *n2, Node *n3, Node *n4) : NodeWithChildren<3>(OpFree, eNodeFor, n1, n2, n3), _child4(n4) {
		NPRINT("NodeFor(%p, %p, %p, <e4>) => %p\n", n1, n2, n3, this);
        n4->ref();
	}
	virtual ~NodeFor() { _child4->unref(); }
protected:
	virtual Node*		doExct();
};

class NodeWhile : public NodeWithChildren<2>
{
public:
	NodeWhile(Node *n1, Node *n2) : NodeWithChildren<2>(OpFree, eNodeWhile, n1, n2) {
		NPRINT("NodeWhile(%p, %p) => %p\n", n1, n2, this);
	}
protected:
	virtual Node*		doExct();
};

class NodeTernary : public NodeWithChildren<3>
{
public:
    NodeTernary(Node *n1, Node *n2, Node *n3) : NodeWithChildren<3>(OpFree, eNodeTernary, n1, n2, n3) {
        NPRINT("NodeTernary(%p, %p, %p) => %p\n", n1, n2, n3, this);
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

// The switch clauses share one contract: handed the switch value in value(), when exct'd they test,
// run their body on a match, and leave a boolean match-result in value().  NodeCaseClauseList and
// NodeCaseLabelList compose two such match-reporters, so the whole tree evaluates via exct() alone.

// child(0) = switch condition; child(1) = clause tree (a clause or a NodeCaseClauseList).
class NodeSwitch : public NodeWithChildren<2>
{
public:
	NodeSwitch(Node *condition, Node *clauses) : NodeWithChildren<2>(OpFree, eNodeSwitch, condition, clauses) {
		NPRINT("NodeSwitch(%p, %p) => %p\n", condition, clauses, this);
	}
protected:
	virtual Node*		doExct();
};

// child(0) = earlier clauses, child(1) = this clause.  First match wins (tests child(0) first).
class NodeCaseClauseList : public NodeWithChildren<2>
{
public:
	NodeCaseClauseList(Node *earlier, Node *clause) : NodeWithChildren<2>(OpFree, eNodeCaseClauseList, earlier, clause) {
		NPRINT("NodeCaseClauseList(%p, %p) => %p\n", earlier, clause, this);
	}
protected:
	virtual Node*		doExct();
};

// child(0) = label matcher (a NodeCaseLabel or NodeCaseLabelList), child(1) = body block.
class NodeCaseClause : public NodeWithChildren<2>
{
public:
	NodeCaseClause(Node *labels, Node *body) : NodeWithChildren<2>(OpFree, eNodeCaseClause, labels, body) {
		NPRINT("NodeCaseClause(%p, %p) => %p\n", labels, body, this);
	}
protected:
	virtual Node*		doExct();
};

// child(0) = earlier labels, child(1) = this label.  Matches if either matches (label grouping).
class NodeCaseLabelList : public NodeWithChildren<2>
{
public:
	NodeCaseLabelList(Node *earlier, Node *label) : NodeWithChildren<2>(OpFree, eNodeCaseLabelList, earlier, label) {
		NPRINT("NodeCaseLabelList(%p, %p) => %p\n", earlier, label, this);
	}
protected:
	virtual Node*		doExct();
};

// A single 'case <expression>:' label.  Reports a match when its expression equals the switch value.
class NodeCaseLabel : public Node1Child
{
public:
	NodeCaseLabel(Node *expression) : Node1Child(OpFree, eNodeCaseLabel, expression) {
		NPRINT("NodeCaseLabel(%p) => %p\n", expression, this);
	}
protected:
	virtual Node*		doExct();
};

// A 'default: { body }' clause.  child(0) = body block.  Always matches; grammar puts it last.
class NodeDefaultClause : public Node1Child
{
public:
	NodeDefaultClause(Node *body) : Node1Child(OpFree, eNodeDefaultClause, body) {
		NPRINT("NodeDefaultClause(%p) => %p\n", body, this);
	}
protected:
	virtual Node*		doExct();
};

#endif /* defined(RT_NODE_H) */
