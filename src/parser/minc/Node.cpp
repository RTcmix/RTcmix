/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* A revised MinC, supporting lists, types and other fun things.
   Based heavily on the classic cmix version by Lars Graf.
   Doug Scott added the '#' and '//' comment parsing.

   John Gibson <johgibso at indiana dot edu>, 1/20/04
 
   Major rewrite to convert entire parser to "real" C++ classes.
 
   Doug Scott, 11/2016 - 04/2017
*/

/* This file holds the intermediate tree representation. */

#define DEBUG
#define DEBUG_TRACE 1  /* if defined to 1, basic trace.  If defined to 2, full trace */

#include "Node.h"
#include "handle.h"
#include <Option.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <rtdefs.h>

extern "C" {
	void yyset_lineno(int line_number);
	int yyget_lineno(void);
};

/* We maintain a stack of MAXSTACK lists, which we access when forming 
   user lists (i.e., {1, 2, "foo"}) and function argument lists.  Each
   element of this stack is a list, allocated and freed by push_list and
   pop_list.  <list> is an array of MincValue structures, each having
   a type and a value, which is encoded in a MincValue union.  Nested lists
   and lists of mixed types are possible.
*/
static MincValue *sMincList;
static int sMincListLen;
static MincValue *list_stack[MAXSTACK];
static int list_len_stack[MAXSTACK];
static int list_stack_ptr;

static StructType *sNewStructType;  // struct currently being defined

static int sArgListLen;		// number of arguments passed to a user-declared function
static int sArgListIndex;	// used to walk passed-in args for user-declared functions

static bool inCalledFunctionArgList = false;
static const char *sCalledFunction;
static int sFunctionCallDepth = 0;	// level of actively-executing function calls

static bool inFunctionCall() { return sFunctionCallDepth > 0; }

static void copy_tree_listelem(MincValue *edest, Node *  tpsrc);
#ifdef DEBUG
static void print_symbol(Symbol * s);		// TODO: Symbol::print()
#endif

void clear_node_state()	// The only exported function from Node.cpp.  Clear all static state.
{
	sMincListLen = 0;
	sMincList = NULL;
	for (int n = 0; n < MAXSTACK; ++n) {
		list_stack[n] = NULL;
		list_len_stack[n] = 0;
	}
	list_stack_ptr = 0;
	inCalledFunctionArgList = false;
	sCalledFunction = NULL;
	sFunctionCallDepth = 0;
	sArgListLen = 0;
	sArgListIndex = 0;
}

#if defined(DEBUG_TRACE)
class Trace {
public:
	Trace(const char *func) : mFunc(func) {
		rtcmix_print("%s%s -->\n", spaces, mFunc);
		++sTraceDepth;
		for (int n =0; n<sTraceDepth*3; ++n) { spaces[n] = ' '; }
		spaces[sTraceDepth*3] = '\0';
	}
	static char *getBuf() { return sMsgbuf; }
	static void printBuf() { rtcmix_print("%s%s", spaces, sMsgbuf); }
	~Trace() {
		 --sTraceDepth;
		for (int n =0; n<sTraceDepth*3; ++n) { spaces[n] = ' '; }
		spaces[sTraceDepth*3] = '\0';
		rtcmix_print("%s<-- %s\n", spaces, mFunc);
	}
private:
	static char sMsgbuf[];
	const char *mFunc;
	static int sTraceDepth;
	static char spaces[];
};
	
char Trace::sMsgbuf[256];
int Trace::sTraceDepth = 0;
char Trace::spaces[128];

#if DEBUG_TRACE==2
#ifdef __GNUC__
#define ENTER() Trace __trace__(__PRETTY_FUNCTION__)
#else
#define ENTER() Trace __trace__(__FUNCTION__)
#endif
#else
#define ENTER()
#endif

#define TPRINT(...) do { snprintf(Trace::getBuf(), 256, __VA_ARGS__); Trace::printBuf(); } while(0)
#else
#define ENTER()
#define TPRINT(...)
#endif

#ifdef DEBUG_MEMORY
#define MPRINT(...) rtcmix_print(__VA_ARGS__)
static int numNodes = 0;
#else
#define MPRINT(...)
#endif

static const char *s_NodeKinds[] = {
   "NodeZero",
   "NodeSeq",
   "NodeStore",
   "NodeList",
   "NodeListElem",
   "NodeEmptyListElem",
   "NodeSubscriptRead",
   "NodeSubscriptWrite",
   "NodeMember",
   "NodeOpAssign",
   "NodeName",
   "NodeAutoName",
   "NodeConstf",
   "NodeString",
   "NodeMemberDecl",
   "NodeStructDef",
   "NodeFuncDef",
   "NodeArgList",
   "NodeArgListElem",
   "NodeRet",
   "NodeFuncSeq",
   "NodeCall",
   "NodeAnd",
   "NodeOr",
   "NodeOperator",
   "NodeUnaryOperator",
   "NodeNot",
   "NodeRelation",
   "NodeIf",
   "NodeWhile",
   "NodeFor",
   "NodeIfElse",
   "NodeDecl",
   "NodeStructDecl",
   "NodeFuncDecl",
   "NodeBlock",
   "NodeNoop"
};

static const char *s_OpKinds[] = {
	"ILLEGAL",
	"ILLEGAL",
	"+",
	"-",
	"*",
	"/",
	"%",
	"^",
	"-",
	"==",
	"!=",
	"<",
	">",
	"<=",
	">="
};

static const char *printNodeKind(NodeKind k)
{
	return s_NodeKinds[k];
}

static const char *printOpKind(OpKind k)
{
	return s_OpKinds[k];
}

/* prototypes for local functions */
static int cmp(MincFloat f1, MincFloat f2);
static void push_list(void);
static void pop_list(void);

/* floating point comparisons:
     f1 < f2   ==> -1
     f1 == f2  ==> 0
     f1 > f2   ==> 1 
*/
static int
cmp(MincFloat f1, MincFloat f2)
{
   if (fabs((double) f1 - (double) f2) < EPSILON) {
      /* printf("cmp=%g %g %g \n",f1,f2,fabs(f1-f2)); */
      return 0;
   }
   if ((f1 - f2) > EPSILON) { 
      /* printf("cmp > %g %g %g \n",f1,f2,fabs(f1-f2)); */
      return 1;
   }
   if ((f2 - f1) > EPSILON) {
      /* printf("cmp <%g %g %g \n",f1,f2,fabs(f1-f2)); */
      return -1;
   }
   return 0;
}

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
    memberSym->next = _memberList;
    _memberList = memberSym;
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

/* ========================================================================== */
/* MincValue */

MincValue::MincValue(MincHandle h) : type(MincHandleType)
{
#ifdef DEBUG_MEMORY
//	MPRINT("created MincValue %p (for MincHandle)\n", this);
#endif
	_u.handle = h; ref_handle(h);
}

MincValue::MincValue(MincList *l) : type(MincListType)
{
#ifdef DEBUG_MEMORY
//	MPRINT("created MincValue %p (for MincList *)\n", this);
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
//	MPRINT("created MincValue %p (for MincDataType)\n", this);
#endif
	_u.list = NULL;		// to zero our contents
}

MincValue::~MincValue()
{
#ifdef DEBUG_MEMORY
//	MPRINT("deleting MincValue %p\n", this);
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
//	MPRINT("\tdone\n");
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
				unref_handle(_u.handle);	// overwriting handle, so unref
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
	ref_handle(h);	// ref before unref
	doClear();
	type = MincHandleType;
	_u.handle = h;
	return *this;
}

const MincValue& MincValue::operator = (MincList *l)
{
	RefCounted::ref(l);	// ref before unref
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

/* ========================================================================== */
/* Tree nodes */

Node::Node(OpKind op, NodeKind kind)
	: kind(kind), op(op), lineno(yyget_lineno())
{
	TPRINT("Node::Node (%s) this=%p\n", classname(), this);
#ifdef DEBUG_MEMORY
	++numNodes;
	TPRINT("[%d nodes in existence]\n", numNodes);
#endif
}

Node::~Node()
{
#ifdef DEBUG_MEMORY
	TPRINT("entering ~Node (%s) this=%p\n", classname(), this);
	--numNodes;
	TPRINT("[%d nodes remaining]\n", numNodes);
#endif
}

const char * Node::classname() const
{
	return printNodeKind(kind);
}

void Node::print()
{
#ifdef DEBUG
	rtcmix_print("Node %p: %s type: %d\n", this, printNodeKind(this->kind), this->dataType());
	if (kind == eNodeName) {
		rtcmix_print("Symbol:\n");
		print_symbol(u.symbol);
	}
	else if (this->dataType() == MincVoidType && child(0) != NULL) {
		rtcmix_print("Child 0:\n");
		child(0)->print();
	}
#endif
}

Node *	Node::exct()
{
	ENTER();
	TPRINT("%s::exct() this=%p\n", classname(), this);
	if (inFunctionCall() && lineno > 0) {
		yyset_lineno(lineno);
	}
	Node *outNode = doExct();	// this is redefined on all subclasses
    TPRINT("%s::exct() done: outNode=%p of type %s\n", classname(), outNode, MincTypeName(outNode->dataType()));
	return outNode;
}

/* This copies a node's value and handles ref counting when necessary */
Node *
Node::copyValue(Node *source)
{
    TPRINT("Node::copyValue(this=%p, %p)\n", this, source);
#ifdef EMBEDDED
    /* Not yet handling nonfatal errors with throw/catch */
    if (tpsrc->dataType() == MincVoidType) {
        return;
    }
#endif
    if (dataType() != MincVoidType && source->dataType() != dataType()) {
        minc_warn("Overwriting %s variable '%s' with %s", MincTypeName(dataType()), name(), MincTypeName(source->dataType()));
    }
    value() = source->value();
    TPRINT("dest: ");
    value().print();
    return this;
}

/* This copies a Symbol's value and handles ref counting when necessary */
Node *
Node::copyValue(Symbol *src)
{
    TPRINT("Node::copyValue(this=%p, %p)\n", this, src);
    assert(src->scope != -1);    // we accessed a variable after leaving its scope!
    if (dataType() != MincVoidType && src->dataType() != dataType()) {
        minc_warn("Overwriting %s variable '%s' with %s", MincTypeName(dataType()), name(), MincTypeName(src->dataType()));
    }
    value() = src->value();
    TPRINT("dest: ");
    value().print();
    return this;
}


NodeNoop::~NodeNoop() {}	// to make sure there is a vtable

/* ========================================================================== */
/* Operators */

/* ---------------------------------------------------------- do_op_string -- */
Node *	NodeOp::do_op_string(const char *str1, const char *str2, OpKind op)
{
	ENTER();
   char *s;
   unsigned long   len;

   switch (op) {
      case OpPlus:   /* concatenate */
         len = (strlen(str1) + strlen(str2)) + 1;
         s = (char *) emalloc(sizeof(char) * len);
         if (s == NULL)
            return NULL;	// TODO: check this
         strcpy(s, str1);
         strcat(s, str2);
         this->v = s;
         // printf("str1=%s, str2=%s len=%d, s=%s\n", str1, str2, len, s);
         break;
      case OpMinus:
      case OpMul:
      case OpDiv:
      case OpMod:
      case OpPow:
      case OpNeg:
		minc_warn("invalid operator for two strings");
		this->v = (char *)NULL;	// TODO: check
		return this;				// TODO: check
      default:
         minc_internal_error("invalid string operator");
         break;
   }
	return this;
}


/* ------------------------------------------------------------- do_op_num -- */
Node *	NodeOp::do_op_num(const MincFloat val1, const MincFloat val2, OpKind op)
{
	ENTER();
   switch (op) {
      case OpPlus:
         this->v = val1 + val2;
         break;
      case OpMinus:
         this->v = val1 - val2;
         break;
      case OpMul:
         this->v = val1 * val2;
         break;
      case OpDiv:
         this->v = val1 / val2;
         break;
      case OpMod:
           if (val2 < 1.0 && val2 > -1.0) {
               minc_die("Illegal value for RHS of a modulo operation");
               this->v = 0.0;
           }
           else {
               this->v = (MincFloat) ((long) val1 % (long) val2);
           }
         break;
      case OpPow:
         this->v = pow(val1, val2);
         break;
      case OpNeg:
         this->v = -val1;        /* <val2> ignored */
         break;
      default:
         minc_internal_error("invalid numeric operator");
         break;
   }
	return this;
}


/* ------------------------------------------------------ do_op_handle_num -- */
Node *	NodeOp::do_op_handle_num(const MincHandle val1, const MincFloat val2,
      OpKind op)
{
	ENTER();
   switch (op) {
      case OpPlus:
      case OpMinus:
      case OpMul:
      case OpDiv:
      case OpMod:
      case OpPow:
         this->v = minc_binop_handle_float(val1, val2, op);
         break;
      case OpNeg:
         this->v = minc_binop_handle_float(val1, -1.0, OpMul);	// <val2> ignored
         break;
      default:
		   // TODO: what should this return on error?
         minc_internal_error("invalid operator for handle and number");
         break;
   }
	return this;
}


/* ------------------------------------------------------ do_op_num_handle -- */
Node *	NodeOp::do_op_num_handle(const MincFloat val1, const MincHandle val2,
      OpKind op)
{
	ENTER();
   switch (op) {
      case OpPlus:
      case OpMinus:
      case OpMul:
      case OpDiv:
      case OpMod:
      case OpPow:
         this->v = minc_binop_float_handle(val1, val2, op);
         break;
      case OpNeg:
         /* fall through */
      default:
		   // TODO: what should this return on error?
         minc_internal_error("invalid operator for handle and number");
         break;
   }
	return this;
}


/* --------------------------------------------------- do_op_handle_handle -- */
Node *	NodeOp::do_op_handle_handle(const MincHandle val1, const MincHandle val2,
      OpKind op)
{
	ENTER();
	switch (op) {
	case OpPlus:
	case OpMinus:
	case OpMul:
	case OpDiv:
	case OpMod:
	case OpPow:
		this->v = minc_binop_handles(val1, val2, op);
		break;
	case OpNeg:
	default:
			// TODO: what should this return on error?
		minc_internal_error("invalid binary handle operator");
		break;
	}
	return this;
}

/* ---------------------------------------------------- do_op_list_iterate -- */
/* Iterate over the list, performing the operation specified by <op>,
   using the scalar <val>, for each list element.  Store the result into a
   new list for <this>, so that child's list is unchanged.
*/
Node *	NodeOp::do_op_list_iterate(const MincList *srcList, const MincFloat val, const OpKind op)
{
	ENTER();
   int i;
   MincValue *dest;
   const int len = srcList->len;
   MincValue *src = srcList->data;
   MincList *destList = new MincList(len);
   dest = destList->data;
   assert(len >= 0);
   switch (op) {
      case OpPlus:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat)src[i] + val;
            else
               dest[i] = src[i];
         }
         break;
      case OpMinus:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat)src[i] - val;
            else
				dest[i] = src[i];
         }
         break;
      case OpMul:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat)src[i] * val;
            else
				dest[i] = src[i];
         }
         break;
      case OpDiv:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat)src[i] / val;
            else
				dest[i] = src[i];
         }
         break;
      case OpMod:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat) (long((MincFloat)src[i]) % (long) val);
            else
				dest[i] = src[i];
         }
         break;
      case OpPow:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = (MincFloat) pow((MincFloat) src[i], (double) val);
            else
				dest[i] = src[i];
         }
         break;
      case OpNeg:
         for (i = 0; i < len; i++) {
            if (src[i].dataType() == MincFloatType)
               dest[i] = -1 * (MincFloat)src[i];    /* <val> ignored */
            else
				dest[i] = src[i];
         }
         break;
      default:
         for (i = 0; i < len; i++)
            dest[i] = 0.0;
         minc_internal_error("invalid list operator");
         break;
   }
   this->value() = destList;
   return this;
}

/* ---------------------------------------------------- do_op_list_list -- */
/* Currently just supports + and +=, concatenating the lists.  Store the result into a
 new list for <this>, so that child's list is unchanged.  N.B. This will operate on zero-length
 and NULL lists as well.
 */
Node *	NodeOp::do_op_list_list(const MincList *list1, const MincList *list2, const OpKind op)
{
	ENTER();
	int i, n;
	const int len1 = (list1) ? list1->len : 0;
	MincValue *src1 = (list1) ? list1->data : NULL;
	const int len2 = (list2) ? list2->len : 0;
	MincValue *src2 = (list2) ? list2->data : NULL;
	
	MincList *destList;
	MincValue *dest;
	switch (op) {
		case OpPlus:
			destList = new MincList(len1+len2);
			dest = destList->data;
			for (i = 0, n = 0; i < len1; ++i, ++n) {
				dest[i] = src1[n];
			}
			for (n = 0; n < len2; ++i, ++n) {
				dest[i] = src2[n];
			}
			break;
		default:
			minc_warn("invalid operator for two lists");
			destList = new MincList(0);		// return zero-length list
			break;
	}
	this->value() = destList;
	destList->ref();
	return this;
}

/* ========================================================================== */
/* Node execution and disposal */

Node *	NodeConstf::doExct()
{
	v = (MincFloat) u.number;
	return this;
}

Node *	NodeString::doExct()
{
	v = u.string;
	return this;
}

Node *	NodeName::doExct()
{
	/* look up the symbol */
	setSymbol(lookupSymbol(_symbolName, AnyLevel));
	return finishExct();
}

Node *	NodeName::finishExct()
{
    Symbol *nodeSymbol;
	if ((nodeSymbol = symbol()) != NULL) {
		TPRINT("%s: symbol %p\n", classname(), nodeSymbol);
		/* For now, symbols for functions cannot be an RHS */
		if (nodeSymbol->node() != NULL) {
			minc_die("Cannot use function '%s' as a variable", symbolName());
		}
		else {
			/* also assign the symbol's value into tree's value field */
			TPRINT("NodeName/NodeAutoName: copying value from symbol '%s' to us\n", nodeSymbol->name());
			copyValue(nodeSymbol);
		}
	}
	else {
		// FIXME: install id w/ value of 0, then warn??
		minc_die("'%s' is not declared", symbolName());
//		return NULL;	// FIX ME: return NULL?  Void Node?
	}
	return this;
}

Node *	NodeAutoName::doExct()
{
	/* look up the symbol */
	setSymbol(lookupOrAutodeclare(symbolName(), sFunctionCallDepth > 0 ? YES : NO));
	return finishExct();
}

Node *	NodeListElem::doExct()
{
	TPRINT("NodeListElem exct'ing Node link %p\n", child(0));
	child(0)->exct();
	if (sMincListLen == MAXDISPARGS) {
		minc_die("exceeded maximum number of items for a list");
		return this;	// TODO: handle no-die case
	}
	else {
		TPRINT("NodeListElem %p evaluating payload child Node %p\n", this, child(1));
		Node * tmp = child(1)->exct();
		/* Copy entire MincValue union from expr to this and to stack. */
		TPRINT("NodeListElem %p copying child value into self and stack\n", this);
		copyValue(tmp);
		copy_tree_listelem(&sMincList[sMincListLen], tmp);
		sMincListLen++;
		TPRINT("NodeListElem: list at level %d now len %d\n", list_stack_ptr, sMincListLen);
	}
	return this;
}

Node *	NodeList::doExct()
{
	push_list();
	child(0)->exct();     /* NB: increments sMincListLen */
	MincList *theList = new MincList(sMincListLen);
	this->v = theList;
	TPRINT("MincList %p assigned to self\n", theList);
	// Copy from stack list into tree list.
    for (int i = 0; i < sMincListLen; ++i) {
		theList->data[i] = sMincList[i];
    }
	pop_list();
	return this;
}

Node *	NodeSubscriptRead::doExct()	// was exct_subscript_read()
{
	ENTER();
	child(0)->exct();         /* lookup target */
	child(1)->exct();         /* index */
	if (child(1)->dataType() != MincFloatType) {
		minc_die("list index must be a number");
		return this;
	}
	MincFloat fltindex = (MincFloat) child(1)->value();
	int index = (int) fltindex;
	MincFloat frac = fltindex - index;
    MincDataType child0Type = child(0)->symbol()->dataType();
    if (child0Type == MincListType) {
        MincList *theList = (MincList *) child(0)->symbol()->value();
        if (theList == NULL) {
            minc_die("attempt to index a NULL list");
            return this;
        }
        int len = theList->len;
        if (len == 0) {
            minc_die("attempt to index an empty list");
            return this;
        }
        if (fltindex < 0.0) {    /* -1 means last element */
            if (fltindex <= -2.0)
                minc_warn("negative index: returning last element");
            index = len - 1;
            frac = 0;
        }
        else if (fltindex > (MincFloat) (len - 1)) {
            minc_warn("attempt to index past the end of list: returning last element");
            index = len - 1;
            frac = 0;
        }
        MincValue elem;
        elem = theList->data[index];

        /* do linear interpolation for float items */
        if (elem.dataType() == MincFloatType && frac > 0.0 && index < len - 1) {
            MincValue& elem2 = theList->data[index + 1];
            if (elem2.dataType() == MincFloatType) {
                value() = (MincFloat) elem
                + (frac * ((MincFloat) elem2 - (MincFloat) elem));
            }
            else { /* can't interpolate btw. a number and another type */
                value() = (MincFloat) elem;
            }
        }
        else {
            this->setValue(elem);
        }
    }
    else if (child0Type == MincStringType) {
        MincString theString = (MincString) child(0)->symbol()->value();
        if (theString == NULL) {
            minc_die("attempt to index a NULL string");
            return this;
        }
        int stringLen = (int)strlen(theString);
        if (index < 0) {    /* -1 means last element */
            if (index <= -2)
                minc_warn("negative index: returning last character");
            index = stringLen - 1;
        }
        else if (index > stringLen - 1) {
            minc_warn("attempt to index past the end of string: returning last element");
            index = stringLen - 1;
        }
        char stringChar[2];
        stringChar[1] = '\0';
        strncpy(stringChar, &theString[index], 1);
        MincValue elem((MincString)strdup(stringChar));  // create new string value from the one character
        this->setValue(elem);
    }
    else {
        minc_die("attempt to index an R-variable that's not a string or list");
    }
	return this;
}

Node *	NodeSubscriptWrite::doExct()	// was exct_subscript_write()
{
	ENTER();
	child(0)->exct();         /* lookup target */
	child(1)->exct();         /* index */
	child(2)->exct();         /* expression to store */
	if (child(1)->dataType() != MincFloatType) {
		minc_die("list index must be a number");
		return this;	// TODO
	}
	if (child(0)->symbol()->dataType() != MincListType) {
		minc_die("attempt to index an L-variable that's not a list");
		return this;	// TODO
	}
	int len = 0;
	MincList *theList = (MincList *) child(0)->symbol()->value();
	MincFloat fltindex = (MincFloat) child(1)->value();
	int index = (int) fltindex;
	if (fltindex - (MincFloat) index > 0.0)
		minc_warn("list index must be integer ... correcting");
	if (theList != NULL) {
		len = theList->len;
		assert(len >= 0);    /* NB: okay to have zero-length list */
	}
    if (index < 0) {    /* means last element */
        if (index <= -2)
            minc_warn("negative index ... assigning to last element");
        index = len > 0 ? len - 1 : 0;
    }
	if (index >= len) {
		/* resize list */
		int newslots;
		newslots = len > 0 ? (index - (len - 1)) : index + 1;
		len += newslots;
		if (len < 0) {
			minc_die("list array subscript exceeds integer size limit!");
		}
		if (theList == NULL) {
			child(0)->symbol()->value() = theList = new MincList(len);
		}
		else
			theList->resize(len);
		TPRINT("exct_subscript_write: MincList %p expanded to len %d\n",
			   theList->data, len);
	}
	copy_tree_listelem(&theList->data[index], child(2));
	copyValue(child(2));
	return this;
}

Node *  NodeMember::doExct()
{
    ENTER();
    child(0)->exct();         /* lookup target */
    Symbol *structSymbol = child(0)->symbol();
    Symbol *memberSymbol = structSymbol->getStructMember(_memberName);
    if (memberSymbol) {
        setSymbol(memberSymbol);
        /* also assign the symbol's value into tree's value field */
        TPRINT("NodeName/NodeAutoName: copying value from member symbol '%s' to us\n", memberSymbol->name());
        copyValue(memberSymbol);
    }
    else {
        minc_die("variable has no member '%s'", _memberName);
    }
    return this;
}

Node *	NodeCall::doExct()
{
	push_list();
	Symbol *funcSymbol = lookupSymbol(_functionName, GlobalLevel);
	if (funcSymbol) {
		sCalledFunction = _functionName;
		/* The function's definition node was stored on the symbol at declaration time.
            If a function was called on a non-function symbol, the tree will be NULL.
		 */
		Node * funcDef = funcSymbol->node();
		if (funcDef) {
			TPRINT("NodeCall: func def = %p\n", funcDef);
			TPRINT("NodeCall: exp decl list = %p\n", child(0));
			child(0)->exct();	// execute arg expression list
			push_function_stack();
			push_scope();
			int savedLineNo, savedScope, savedCallDepth;
			Node * temp = NULL;
			try {
				/* The exp list is copied to the symbols for the function's arg list. */
				funcDef->child(1)->exct();
				savedLineNo = yyget_lineno();
				savedScope = current_scope();
				++sFunctionCallDepth;
				savedCallDepth = sFunctionCallDepth;
				TPRINT("NodeCall(%p): executing %s() block node %p, call depth now %d\n",
					   this, sCalledFunction, funcDef->child(2), savedCallDepth);
				printargs(sCalledFunction, NULL, 0);
				temp = funcDef->child(2)->exct();
			}
			catch (Node * returned) {	// This catches return statements!
				TPRINT("NodeCall(%p) caught %p return stmt throw - restoring call depth %d\n",
					   this, returned, savedCallDepth);
				temp = returned;
				sFunctionCallDepth = savedCallDepth;
				restore_scope(savedScope);
			}
			catch(...) {	// Anything else is an error
				pop_function_stack();
				--sFunctionCallDepth;
				throw;
			}
			--sFunctionCallDepth;
			TPRINT("NodeCall: function call depth => %d\n", sFunctionCallDepth);
			// restore parser line number
			yyset_lineno(savedLineNo);
			TPRINT("NodeCall copying def exct results into self\n");
			copyValue(temp);
			pop_function_stack();
		}
		else {
			minc_die("'%s' is not a function", funcSymbol->name());
		}
		sCalledFunction = NULL;
	}
	else {
		child(0)->exct();
		MincValue retval;
		int result = call_builtin_function(_functionName, sMincList, sMincListLen,
										   &retval);
		if (result == FUNCTION_NOT_FOUND) {
			result = call_external_function(_functionName, sMincList, sMincListLen,
											&retval);
		}
		this->setValue(retval);
		switch (result) {
            case NO_ERROR:
                break;
            case FUNCTION_NOT_FOUND:
#if defined(EMBEDDED) && defined(ERROR_FAIL_ON_UNDEFINED_FUNCTION)
                throw result;
#endif
                break;
            default:
#if defined(EMBEDDED)
                throw result;
#endif
                break;
		}
	}
	pop_list();
	return this;
}

Node *	NodeStore::doExct()
{
#ifdef ORIGINAL_CODE
	/* N.B. Now that symbol lookup is part of tree, this happens in
	 the NodeName stored as child[0] */
	TPRINT("NodeStore(%p): evaluate LHS %p (child 0)\n", this, child(0));
	child(0)->exct();
	/* evaluate RHS expression */
	TPRINT("NodeStore(%p): evaluate RHS (child 1)\n", this);
	child(1)->exct();
#else
	/* evaluate RHS expression */
	TPRINT("NodeStore(%p): evaluate RHS (child 1) FIRST\n", this);
	child(1)->exct();
	/* N.B. Now that symbol lookup is part of tree, this happens in
	 the NodeName stored as child[0] */
	TPRINT("NodeStore(%p): evaluate LHS %p (child 0)\n", this, child(0));
	child(0)->exct();
#endif
	TPRINT("NodeStore(%p): copying value from RHS (%p) to LHS's symbol (%p)\n",
		   this, child(1), child(0)->symbol());
	/* Copy entire MincValue union from expr to id sym and to this. */
	child(0)->symbol()->copyValue(child(1));
	TPRINT("NodeStore: copying value from RHS (%p) to here (%p)\n", child(1), this);
	copyValue(child(1));
	return this;
}

Node *	NodeOpAssign::doExct()		// was exct_opassign()
{
	ENTER();
	Node *tp0 = child(0)->exct();
	Node *tp1 = child(1)->exct();
	
	if (tp0->symbol()->dataType() != MincFloatType || tp1->dataType() != MincFloatType) {
		minc_warn("can only use '%c=' with numbers",
				  op == OpPlus ? '+' : (op == OpMinus ? '-'
										: (op == OpMul ? '*' : '/')));
		copyValue(tp0->symbol());
		return this;
	}
	MincValue& symValue = tp0->symbol()->value();
	MincFloat rhs = (MincFloat)tp1->value();
	switch (this->op) {
		case OpPlus:
			symValue = (MincFloat)symValue + rhs;
			break;
		case OpMinus:
			symValue = (MincFloat)symValue - rhs;
			break;
		case OpMul:
			symValue = (MincFloat)symValue * rhs;
			break;
		case OpDiv:
			symValue = (MincFloat)symValue / rhs;
			break;
		default:
			minc_internal_error("exct: tried to execute invalid NodeOpAssign");
			break;
	}
	this->value() = tp0->symbol()->value();
	return this;
}

Node *	NodeNot::doExct()
{
	if (cmp(0.0, (MincFloat)child(0)->exct()->value()) == 0)
		this->value() = 1.0;
	else
		this->value() = 0.0;
	return this;
}

Node *	NodeAnd::doExct()
{
	this->value() = 0.0;
	if (cmp(0.0, (MincFloat)child(0)->exct()->value()) != 0) {
		if (cmp(0.0, (MincFloat)child(1)->exct()->value()) != 0) {
			this->value() = 1.0;
		}
	}
	return this;
}

Node *	NodeRelation::doExct()		// was exct_relation()
{
	ENTER();
	MincValue& v0 = child(0)->exct()->value();
	MincValue& v1 = child(1)->exct()->value();
	
	if (v0.dataType() != v1.dataType()) {
		minc_warn("operator %s: attempt to compare variables having different types", printOpKind(this->op));
		this->value() = 0.0;
		return this;
	}
	
	switch (this->op) {
		case OpEqual:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) == 0)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				case MincStringType:
					if (strcmp((MincString)v0, (MincString)v1) == 0)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		case OpNotEqual:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) == 0)
						this->value() = 0.0;
					else
						this->value() = 1.0;
					break;
				case MincStringType:
					if (strcmp((MincString)v0, (MincString)v1) == 0)
						this->value() = 0.0;
					else
						this->value() = 1.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		case OpLess:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) == -1)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		case OpGreater:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) == 1)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		case OpLessEqual:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) <= 0)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		case OpGreaterEqual:
			switch (v0.dataType()) {
				case MincFloatType:
					if (cmp((MincFloat)v0, (MincFloat)v1) >= 0)
						this->value() = 1.0;
					else
						this->value() = 0.0;
					break;
				default:
					goto unsupported_type;
					break;
			}
			break;
		default:
			minc_internal_error("exct: tried to execute invalid NodeRelation");
			break;
	}
	return this;
unsupported_type:
	minc_internal_error("operator %s: can't compare this type of object", printOpKind(this->op));
	return this;	// TODO
}

Node *	NodeOp::doExct()
{
	ENTER();
	MincValue& v0 = child(0)->exct()->value();
	MincValue& v1 = child(1)->exct()->value();
	switch (v0.dataType()) {
		case MincFloatType:
			switch (v1.dataType()) {
				case MincFloatType:
					do_op_num((MincFloat)v0, (MincFloat)v1, this->op);
					break;
				case MincStringType:
				{
					char buf[64];
					snprintf(buf, 64, "%g", (MincFloat)v0);
					do_op_string(buf, (MincString)v1, this->op);
				}
					break;
				case MincHandleType:
					do_op_num_handle((MincFloat)v0, (MincHandle)v1, this->op);
					break;
				case MincListType:
					/* Check for nonsensical ops. */
					if (this->op == OpMinus)
						minc_warn("can't subtract a list from a number");
					else if (this->op == OpDiv)
						minc_warn("can't divide a number by a list");
					else
						do_op_list_iterate((MincList*)v1, (MincFloat)v0, this->op);
					break;
                case MincStructType:
                    minc_warn("operator %s: a struct cannot be used for this operation", printOpKind(this->op));
                    break;
				default:
					minc_internal_error("operator %s: invalid rhs type: %s", printOpKind(this->op), MincTypeName(v1.dataType()));
					break;
			}
			break;
		case MincStringType:
			switch (v1.dataType()) {
				case MincFloatType:
				{
					char buf[64];
					snprintf(buf, 64, "%g", (MincFloat)v1);
					do_op_string((MincString)v0, buf, this->op);
				}
					break;
				case MincStringType:
					do_op_string((MincString)v0, (MincString)v1, this->op);
					break;
				case MincHandleType:
					minc_warn("can't operate on a string and a handle");
					break;
				case MincListType:
					minc_warn("can't operate on a string and a list");
					break;
                case MincStructType:
                    minc_warn("operator %s: a struct cannot be used for this operation", printOpKind(this->op));
                    break;
				default:
					minc_internal_error("operator %s: invalid rhs type: %s", printOpKind(this->op), MincTypeName(v1.dataType()));
					break;
			}
			break;
		case MincHandleType:
			switch (v1.dataType()) {
				case MincFloatType:
					do_op_handle_num((MincHandle)v0, (MincFloat)v1, this->op);
					break;
				case MincStringType:
					minc_warn("can't operate on a string and a handle");
					break;
				case MincHandleType:
					do_op_handle_handle((MincHandle)v0, (MincHandle)v1, this->op);
					break;
				case MincListType:
					minc_warn("can't operate on a list and a handle");
					break;
                case MincStructType:
                    minc_warn("operator %s: a struct cannot be used for this operation", printOpKind(this->op));
                    break;
				default:
					minc_internal_error("operator %s: invalid rhs type: %s", printOpKind(this->op), MincTypeName(v1.dataType()));
					break;
			}
			break;
		case MincListType:
			switch (v1.dataType()) {
				case MincFloatType:
					do_op_list_iterate((MincList *)v0, (MincFloat)v1, this->op);
					break;
				case MincStringType:
					minc_warn("can't operate on a string");
					break;
				case MincHandleType:
					minc_warn("can't operate on a handle");
					break;
				case MincListType:
					do_op_list_list((MincList *)v0, (MincList *)v1, this->op);
					break;
                case MincStructType:
                    minc_warn("operator %s: a struct cannot be used for this operation", printOpKind(this->op));
                    break;
				default:
					minc_internal_error("operator %s: invalid rhs type: %s", printOpKind(this->op), MincTypeName(v1.dataType()));
					break;
			}
			break;
        case MincStructType:
            minc_warn("operator %s: a struct cannot be used for this operation", printOpKind(this->op));
            break;
		default:
            minc_internal_error("operator %s: invalid lhs type: %s", printOpKind(this->op), MincTypeName(v1.dataType()));
			break;
	}
	return this;
}

Node *	NodeUnaryOperator::doExct()
{
	if (this->op == OpNeg)
		this->value() = -1 * (MincFloat)child(0)->exct()->value();
	return this;
}

Node *	NodeOr::doExct()
{
	this->value() = 0.0;
	if ((cmp(0.0, (MincFloat)child(0)->exct()->value()) != 0) ||
		(cmp(0.0, (MincFloat)child(1)->exct()->value()) != 0)) {
		this->value() = 1.0;
	}
	return this;
}

Node *	NodeIf::doExct()
{
	if (cmp(0.0, (MincFloat)child(0)->exct()->value()) != 0)
		child(1)->exct();
	return this;
}

Node *	NodeIfElse::doExct()
{
	if (cmp(0.0, (MincFloat)child(0)->exct()->value()) != 0)
		child(1)->exct();
	else
		child(2)->exct();
	return this;
}

Node *	NodeWhile::doExct()
{
	while (cmp(0.0, (MincFloat)child(0)->exct()->value()) != 0)
		child(1)->exct();
	return this;
}

Node *	NodeArgList::doExct()
{
	sArgListLen = 0;
	sArgListIndex = 0;	// reset to walk list
	inCalledFunctionArgList = true;
	TPRINT("NodeArgList: walking function '%s()' arg decl/copy list\n", sCalledFunction);
	child(0)->exct();
	inCalledFunctionArgList = false;
	return this;
}

Node *	NodeArgListElem::doExct()
{
	++sArgListLen;
	child(0)->exct();	// work our way to the front of the list
	child(1)->exct();	// run the arg decl
	// Symbol associated with this function argument
	Symbol *argSym = child(1)->symbol();
	if (sMincListLen > sArgListLen) {
		minc_die("%s() takes %d arguments but was passed %d!", sCalledFunction, sArgListLen, sMincListLen);
	}
	else if (sArgListIndex >= sMincListLen) {
		minc_warn("%s(): arg '%s' not provided - defaulting to 0", sCalledFunction, argSym->name());
		/* Copy zeroed MincValue to us and then to sym. */
		MincValue zeroElem;
		zeroElem = argSym->value();	// this captures the data type
		zeroElem.zero();
		this->setValue(zeroElem);
		argSym->copyValue(this);
		++sArgListIndex;
	}
	/* compare stored NodeName with user-passed arg */
	else {
		// Pre-cached argument value from caller
		MincValue &argValue = sMincList[sArgListIndex];
		bool compatible = false;
		switch (argValue.dataType()) {
			case MincFloatType:
			case MincStringType:
			case MincHandleType:
			case MincListType:
            case MincStructType:
				if (argSym->dataType() != argValue.dataType()) {
					minc_die("%s() arg '%s' passed as %s, expecting %s",
								sCalledFunction, argSym->name(), MincTypeName(argValue.dataType()), MincTypeName(argSym->dataType()));
				}
				else compatible = true;
				break;
			default:
				assert(argValue.dataType() != MincVoidType);
				break;
		}
		if (compatible) {
			/* Copy passed-in arg's MincValue union to us and then to sym. */
			this->setValue(argValue);
			argSym->copyValue(this);
		}
		++sArgListIndex;
	}
	return this;
}

Node *	NodeRet::doExct()
{
	child(0)->exct();
	copyValue(child(0));
	TPRINT("NodeRet throwing %p for return stmt\n", this);
	throw this;	// Cool, huh?  Throws this node's body out to function's endpoint!
	return NULL;	// notreached
}

Node *	NodeFuncSeq::doExct()
{
	child(0)->exct();
	child(1)->exct();
	copyValue(child(1));
	return this;
}

Node *	NodeFor::doExct()
{
	child(0)->exct();         /* init */
	while (cmp(0.0, (MincFloat)child(1)->exct()->value()) != 0) { /* condition */
		_child4->exct();      /* execute block */
		child(2)->exct();      /* prepare for next iteration */
	}
	return this;
}

Node *	NodeSeq::doExct()
{
	child(0)->exct();
	child(1)->exct();
	return this;
}

Node *	NodeBlock::doExct()
{
	push_scope();
	child(0)->exct();
	pop_scope();
	return this;				// NodeBlock returns void type
}

Node *	NodeDecl::doExct()
{
	TPRINT("-- declaring variable '%s'\n", _symbolName);
	Symbol *sym = lookupSymbol(_symbolName, inCalledFunctionArgList ? ThisLevel : AnyLevel);
	if (!sym) {
		sym = installSymbol(_symbolName, NO);
		sym->value() = MincValue(this->_type);
	}
	else {
		if (sym->scope == current_scope()) {
			if (inCalledFunctionArgList) {
				minc_die("%s(): argument variable '%s' already used", sCalledFunction, _symbolName);
			}
			minc_warn("variable '%s' redefined - using existing one", _symbolName);
		}
		else {
			if (sFunctionCallDepth == 0) {
				minc_warn("variable '%s' also defined at enclosing scope", _symbolName);
			}
			sym = installSymbol(_symbolName, NO);
			sym->value() = MincValue(this->_type);
		}
	}
	this->setSymbol(sym);
	return this;
}

Node *  NodeStructDef::doExct()
{
    TPRINT("-- storing declaration for struct type '%s'\n", _typeName);
    assert(current_scope() == 0);    // until I allow nested structs
    sNewStructType = installType(_typeName, YES);  // all structs global for now
    if (sNewStructType) {
        TPRINT("-- walking element list\n");
        child(0)->exct();
        sNewStructType = NULL;
    }
    return this;
}

Node *  NodeMemberDecl::doExct()
{
    TPRINT("-- storing decl info for member '%s', type %s\n", _symbolName, MincTypeName(this->_type));
    assert(sNewStructType != NULL);
    sNewStructType->addElement(_symbolName, this->_type);
    return this;
}

Node *    NodeStructDecl::doExct()
{
    TPRINT("-- looking up type '%s'\n", _typeName);
    const StructType *structType = lookupType(_typeName, GlobalLevel);    // GlobalLevel for now
    if (structType) {
        TPRINT("-- declaring variable '%s'\n", _symbolName);
        Symbol *sym = lookupSymbol(_symbolName, GlobalLevel);       // GlobalLevel for now
        if (!sym) {
            sym = installSymbol(_symbolName, YES);          // YES for now
            sym->init(structType);
        }
        else {
            if (sym->scope == current_scope()) {
                if (inCalledFunctionArgList) {
                    minc_die("%s(): argument variable '%s' already used", sCalledFunction, _symbolName);
                }
                minc_warn("variable '%s' redefined - using existing one", _symbolName);
            }
            else {
                if (sFunctionCallDepth == 0) {
                    minc_warn("variable '%s' also defined at enclosing scope", _symbolName);
                }
                sym = installSymbol(_symbolName, NO);
                sym->init(structType);
            }
        }
        this->setSymbol(sym);
    }
    else {
        minc_die("struct type '%s' is not defined", _typeName);
    }
    return this;
}

Node *	NodeFuncDecl::doExct()
{
	TPRINT("-- declaring function '%s'\n", _symbolName);
	assert(current_scope() == 0);	// until I allow nested functions
	Symbol *sym = lookupSymbol(_symbolName, GlobalLevel);	// only look at current global level
	if (sym == NULL) {
		sym = installSymbol(_symbolName, YES);		// all functions global for now
		sym->value() = MincValue(this->_type);
		this->setSymbol(sym);
	}
	else {
#ifdef EMBEDDED
		minc_warn("function %s() is already declared", _symbolName);
		this->setSymbol(sym);
#else
		minc_die("function %s() is already declared", _symbolName);
#endif
	}
	return this;
}

Node *	NodeFuncDef::doExct()
{
	// Look up symbol for function, and bind this FuncDef node to it.
	TPRINT("NodeFuncDef: executing lookup node %p\n", child(0));
	child(0)->exct();
	assert(child(0)->symbol() != NULL);
	child(0)->symbol()->setNode(this);
	return this;
}

static void
push_list()
{
	ENTER();
   if (list_stack_ptr >= MAXSTACK)
      minc_die("stack overflow: too many nested function calls");
   list_stack[list_stack_ptr] = sMincList;
   list_len_stack[list_stack_ptr++] = sMincListLen;
   sMincList = new MincValue[MAXDISPARGS];
   TPRINT("push_list: sMincList=%p at stack level %d, len %d\n", sMincList, list_stack_ptr, sMincListLen);
   sMincListLen = 0;
}


static void
pop_list()
{
	ENTER();
   TPRINT("pop_list: sMincList=%p\n", sMincList);
   delete [] sMincList;
   if (list_stack_ptr == 0)
      minc_die("stack underflow");
   sMincList = list_stack[--list_stack_ptr];
   sMincListLen = list_len_stack[list_stack_ptr];
	TPRINT("pop_list: now at sMincList=%p, stack level %d, len %d\n", sMincList, list_stack_ptr, sMincListLen);
}

static void
copy_tree_listelem(MincValue *dest, Node *tpsrc)
{
   TPRINT("copy_tree_listelem(%p, %p)\n", dest, tpsrc);
#ifdef EMBEDDED
	/* Not yet handling nonfatal errors with throw/catch */
	if (tpsrc->dataType() == MincVoidType) {
		return;
	}
#endif
	*dest = tpsrc->value();
}

#ifdef DEBUG
static void print_symbol(Symbol * s)
{
	rtcmix_print("Symbol %p: '%s' scope: %d type: %d\n", s, s->name(), s->scope, s->dataType());
}
#endif

