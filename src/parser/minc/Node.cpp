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
 
   Major revision to support struct declarations.
 
    Doug Scott, 12/2019
 
   Added new Map type.
 
    Doug Scott, 08/2020
    Doug Scott, 08/2020

   Added embedding of types within types.  Added new 'function pointer' mfunction type.
 
    Doug Scott, 11/2020.
 
   Added real object-oriented support via struct member functions.
 
    Doug Scott, 06/2022.
 
   Added full object-oriented support via structs (methods, etc.)
 
    Doug Scott, 07/2022
   
*/

/* This file holds the intermediate tree representation as a linked set of Nodes. */

#undef DEBUG
#undef DEBUG_FILENAME_INCLUDES /* DAS - I use this for debugging error reporting */
#undef SCOPE_DEBUG  /* DAS - This is for the scope-assist code (level counters, etc.) */

#include "debug.h"

#include "Node.h"
#include "MincValue.h"
#include "Scope.h"
#include "Symbol.h"
#include <RTOption.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

extern "C" {
	void yyset_lineno(int line_number);
	int yyget_lineno(void);
    void yy_store_lineno(int line_number);
    const char * yy_get_current_include_filename();
    void yy_set_current_include_filename(const char *include_file);
};

/* builtin.cpp */
extern int call_builtin_function(const char *funcname, const MincValue arglist[],
                          const int nargs, MincValue *retval);
extern int call_object_method(MincValue &object, const char *methodName, const MincValue arglist[],
                              int nargs, MincValue *retval);

/* callextfunc.cpp */
extern int call_external_function(const char *funcname, const MincValue arglist[],
                           const int nargs, MincValue *return_value);
extern MincHandle minc_binop_handle_float(const MincHandle handle, const MincFloat val, OpKind op);
extern MincHandle minc_binop_float_handle(const MincFloat val, const MincHandle handle, OpKind op);
extern MincHandle minc_binop_handles(const MincHandle handle1, const MincHandle handle2, OpKind op);

#ifdef SCOPE_DEBUG
static char ssBuf[256];
#define SPRINT(...) do { snprintf(ssBuf, 256, __VA_ARGS__); rtcmix_print("%s\n", ssBuf); } while(0)
#else
#define SPRINT(...)
#endif

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

static int sFunctionCallDepth = 0;	// level of actively-executing function calls
static void incrementFunctionCallDepth() { ++sFunctionCallDepth; SPRINT("sFunctionCallDepth -> %d", sFunctionCallDepth); }
static void decrementFunctionCallDepth() { --sFunctionCallDepth; SPRINT("sFunctionCallDepth -> %d", sFunctionCallDepth); }
static bool inFunctionCall() { return sFunctionCallDepth > 0; }

static std::vector<const char *> sCalledFunctions;

// Note:  This counter is a hack to allow backwards-compat scope behavior for if() blocks but allow
// new scope behavior for for() and while() blocks:  The former stays in global score, the latter do not.

static int sForWhileBlockDepth = 0;      // level of actively-executing while() and for() blocks
static void incrementForWhileBlockDepth() { ++sForWhileBlockDepth; SPRINT("sForWhileBlockDepth -> %d", sForWhileBlockDepth); }
static void decrementForWhileBlockDepth() { --sForWhileBlockDepth; SPRINT("sForWhileBlockDepth -> %d", sForWhileBlockDepth); }
static bool inForOrWhileBlock() { return sForWhileBlockDepth > 0; }

static int sIfElseBlockDepth = 0;      // level of actively-executing if() and else() blocks
static void incrementIfElseBlockDepth() { ++sIfElseBlockDepth; SPRINT("sIfElseBlockDepth -> %d", sIfElseBlockDepth); }
static void decrementIfElseBlockDepth() { --sIfElseBlockDepth; SPRINT("sIfElseBlockDepth -> %d", sIfElseBlockDepth); }
static bool inIfOrElseBlock() { return sIfElseBlockDepth > 0; }

static void copyNodeToMincList(MincValue *edest, Node *  tpsrc);

static MincWarningLevel sMincWarningLevel = MincAllWarnings;

// The only exported functions from Node.cpp.

// Clear all static state.
void clear_tree_state()
{
	sMincListLen = 0;
	sMincList = NULL;
	for (int n = 0; n < MAXSTACK; ++n) {
		list_stack[n] = NULL;
		list_len_stack[n] = 0;
	}
	list_stack_ptr = 0;
	inCalledFunctionArgList = false;
	sCalledFunctions.clear();
	sFunctionCallDepth = 0;
	sArgListLen = 0;
	sArgListIndex = 0;
    sNewStructType = NULL;
}

static const char *s_NodeKinds[] = {
   "ILLEGAL",
   "NodeSeq",
   "NodeStore",
   "NodeList",
   "NodeListElem",
   "NodeEmptyListElem",
   "NodeSubscriptRead",
   "NodeSubscriptWrite",
   "NodeSubscriptOpAssign",
   "NodeMemberAccess",
   "NodeOpAssign",
   "NodeLoadSym",
   "NodeAutoDeclLoadSym",
   "NodeConstf",
   "NodeString",
   "NodeMemberDecl",
   "NodeStructDef",
   "NodeFuncDef",
   "NodeMethodDef",
   "NodeArgList",
   "NodeArgListElem",
   "NodeRet",
   "NodeFuncBodySeq",
   "NodeFuncCall",
   "NodeMethodCall",
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
   "NodeMethodDecl",
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
	">=",
    "++",
    "--"
};

static const char *printNodeKind(NodeKind k)
{
	return s_NodeKinds[k];
}

static const char *printOpKind(OpKind k)
{
	return s_OpKinds[k];
}

// WARNING: NOT THREAD SAFE

static const char *methodNameFromStructAndFunction(const char *structName, const char *functionName)
{
    static char sMethodNameBuffer[128];
    snprintf(sMethodNameBuffer, 128, "%s.%s", structName, functionName);
    return strsave(sMethodNameBuffer);
}

/* prototypes for local functions */
static void push_list(void);
static void pop_list(void);

#ifdef DEBUG_NODE_MEMORY
static int numNodes = 0;
#endif

/* ========================================================================== */
/* Tree nodes */

Node::Node(OpKind op, NodeKind kind)
	: kind(kind), op(op), lineno(yyget_lineno()), includeFilename(yy_get_current_include_filename())
{
	NPRINT("Node::Node (%s) this=%p storing lineno %d, includefile '%s'\n", classname(), this, lineno, includeFilename);
#ifdef DEBUG_NODE_MEMORY
	++numNodes;
    NPRINT("[%d nodes in existence]\n", numNodes);
#endif
    u.number = 0.0;     // this should zero out the union.
}

Node::~Node()
{
#ifdef DEBUG_NODE_MEMORY
    NPRINT("entering ~Node (%s) this=%p\n", classname(), this);
	--numNodes;
    NPRINT("[%d nodes remaining]\n", numNodes);
#endif
}

const char * Node::name() const
{
    return symbol() ? symbol()->name() : "UNDEFINED";
}

const char * Node::classname() const
{
	return printNodeKind(kind);
}

void Node::print()
{
    TPRINT("Node %p contents: class: %s type: %s\n", this, classname(), MincTypeName(this->dataType()));
	if (kind == eNodeLoadSym) {
		TPRINT("\tsymbol:\n");
        symbol()->print();
	}
	else if (this->dataType() == MincVoidType && child(0) != NULL) {
		TPRINT("\tchild 0:\n");
		child(0)->print();
	}
    else {
        TPRINT("\tvalue: ");
#if DEBUG_TRACE
        value().print();
#else
        TPRINT("\n");
#endif
    }
}

Node *	Node::exct()
{
	ENTER();
    const char *savedIncludeFilename = includeFilename;
#ifdef DEBUG_FILENAME_INCLUDES
    printf("%s::exct(%p) setting current location to '%s', current_lineno to %d\n", classname(), this, includeFilename, lineno);
#endif
    if (RTOption::parserWarnings() < MincAllWarnings) {
        sMincWarningLevel = MincNoDefaultedArgWarnings;
    }
    yy_store_lineno(lineno);
    yy_set_current_include_filename(includeFilename);
	Node *outNode = doExct();	// this is redefined on all subclasses
    TPRINT("%s::exct(%p) done: returning node %p of type %s\n", classname(), this, outNode, MincTypeName(outNode->dataType()));
#ifdef DEBUG_FILENAME_INCLUDES
    printf("%s::exct(%p) restoring current location to '%s'\n", classname(), this, savedIncludeFilename);
#endif
    yy_set_current_include_filename(savedIncludeFilename);
	return outNode;
}

/* This copies a node's value and handles ref counting when necessary */
Node *
Node::copyValue(Node *source, bool allowTypeOverwrite, bool suppressOverwriteWarning)
{
    TPRINT("Node::copyValue(this=%p, Node=%p)\n", this, source);
#ifdef EMBEDDED
    /* Not yet handling nonfatal errors with throw/catch */
    if (source->dataType() == MincVoidType) {
        return this;
    }
#endif
    if (dataType() != MincVoidType && source->dataType() != dataType()) {
        if (allowTypeOverwrite) {
            if (!suppressOverwriteWarning) {
                minc_warn("Overwriting %s variable '%s' with a %s", MincTypeName(dataType()), name(),
                          MincTypeName(source->dataType()));
            }
        }
        else {
            minc_die("Cannot overwrite '%s' (type %s) with a %s", name(), MincTypeName(dataType()), MincTypeName(source->dataType()));
        }
    }
    setValue(source->value());
#ifdef DEBUG
    TPRINT("\tthis: ");
    print();
#endif
    return this;
}

/* This copies a Symbol's value and handles ref counting when necessary */
Node *
Node::copyValue(Symbol *source, bool allowTypeOverwrite, bool suppressOverwriteWarning)
{
    TPRINT("Node::copyValue(this=%p, Symbol=%p)\n", this, source);
    assert(source->scope() != -1);    // we accessed a variable after leaving its scope!
    if (dataType() != MincVoidType && source->dataType() != dataType()) {
        if (allowTypeOverwrite) {
            if (!suppressOverwriteWarning) {
                minc_warn("Overwriting %s variable '%s' with a %s", MincTypeName(dataType()), name(),
                          MincTypeName(source->dataType()));
            }
        }
        else {
            minc_die("Cannot overwrite '%s' (type %s) with a %s", name(), MincTypeName(dataType()), MincTypeName(source->dataType()));
        }
    }
    setValue(source->value());
#ifdef DEBUG
    TPRINT("\tthis: ");
    print();
#endif
    return this;
}


NodeNoop::~NodeNoop() {}	// to make sure there is a vtable

/* ========================================================================== */
/* Operators - handled by OperationBase, which is used as base class. */

/* ---------------------------------------------------------- do_op_string -- */
Node *	OperationBase::do_op_string(Node *node, const char *str1, const char *str2, OpKind op)
{
	ENTER();
   char *s;
   unsigned long   len;

   switch (op) {
      case OpPlus:   /* concatenate */
         len = (strlen(str1) + strlen(str2)) + 1;
         s = (char *) emalloc(sizeof(char) * len);
         strcpy(s, str1);
         strcat(s, str2);
         node->v = s;
         // printf("str1=%s, str2=%s len=%d, s=%s\n", str1, str2, len, s);
         break;
      case OpMinus:
      case OpMul:
      case OpDiv:
      case OpMod:
      case OpPow:
        minc_warn("invalid '%s' operator for two strings", printOpKind(op));
        node->v = (MincString)NULL;
        break;
      case OpNeg:
		minc_warn("invalid negation of string");
        break;
      default:
         minc_internal_error("invalid string operator");
         break;
   }
	return node;
}


/* ------------------------------------------------------------- do_op_num -- */
Node *	OperationBase::do_op_num(Node *node, const MincFloat val1, const MincFloat val2, OpKind op)
{
	ENTER();
   switch (op) {
      case OpPlus:
          node->v = val1 + val2;
         break;
      case OpMinus:
          node->v = val1 - val2;
         break;
      case OpMul:
          node->v = val1 * val2;
         break;
      case OpDiv:
          if (val2 == 0.0f) {
              minc_warn("Divide-by-zero will return 'inf'!");
          }
          node->v = val1 / val2;
         break;
      case OpMod:
           if (val2 < 1.0 && val2 > -1.0) {
               minc_die("Illegal value for RHS of a modulo operation");
               node->v = 0.0;
           }
           else {
               node->v = (MincFloat) ((long) val1 % (long) val2);
           }
         break;
      case OpPow:
          node->v = pow(val1, val2);
         break;
      case OpNeg:
          node->v = -val1;        /* <val2> ignored */
         break;
      default:
         minc_internal_error("invalid numeric operator");
         break;
   }
	return node;
}


/* ------------------------------------------------------ do_op_handle_num -- */
Node *	OperationBase::do_op_handle_num(Node *node, const MincHandle val1, const MincFloat val2,
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
          node->v = minc_binop_handle_float(val1, val2, op);
         break;
      case OpNeg:
          node->v = minc_binop_handle_float(val1, -1.0, OpMul);	// <val2> ignored
         break;
      default:
         minc_internal_error("invalid '%s' operator for handle and number", printOpKind(op));
         break;
   }
	return node;
}


/* ------------------------------------------------------ do_op_num_handle -- */
Node *	OperationBase::do_op_num_handle(Node *node, const MincFloat val1, const MincHandle val2,
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
         node->v = minc_binop_float_handle(val1, val2, op);
         break;
      case OpNeg:
         /* fall through */
      default:
         minc_internal_error("invalid '%s' operator for number and handle", printOpKind(op));
         break;
   }
	return node;
}


/* --------------------------------------------------- do_op_handle_handle -- */
Node *	OperationBase::do_op_handle_handle(Node *node, const MincHandle val1, const MincHandle val2,
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
		node->v = minc_binop_handles(val1, val2, op);
		break;
	case OpNeg:
	default:
		minc_internal_error("invalid binary handle operator '%s'", printOpKind(op));
		break;
	}
	return node;
}

/* ---------------------------------------------------- do_op_list_float -- */
/* Iterate over the list, performing the operation specified by <op>,
   using the scalar <val>, for each list element - with the element first in the equation.  Store the result into a
   new list for <this>, so that child's list is unchanged.
*/
Node *	OperationBase::do_op_list_float(Node *node, const MincList *srcList, const MincFloat val, const OpKind op)
{
	ENTER();
   int i;
   if (srcList == NULL) {
       minc_warn("Null list in binary operation");
       node->setValue(MincValue((MincList *)NULL));
       return node;
   }
   const int len = srcList->len;
   MincValue *src = srcList->data;
   MincList *destList = new MincList(len);
   MincValue *dest = destList->data;
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
   node->setValue(MincValue(destList));
   return node;
}

/* ---------------------------------------------------- do_op_list_list -- */
/* Currently just supports + and +=, concatenating the lists.  Store the result into a
 new list for <this>, so that child's list is unchanged.  N.B. This will operate on zero-length
 and NULL lists as well.
 */
Node *	OperationBase::do_op_list_list(Node *node, const MincList *list1, const MincList *list2, const OpKind op)
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
			minc_warn("invalid '%s' operator for two lists", printOpKind(op));
			destList = new MincList(0);		// return zero-length list
			break;
	}
    node->setValue(MincValue(destList));
	destList->ref();
	return node;
}

/* ---------------------------------------------------- do_op_float_list -- */
/* Iterate over the list, performing the operation specified by <op>,
 using the scalar <val>, for each list element - with <val> first in the equation.  Store the result into a
 new list for <this>, so that child's list is unchanged.  NOTE:  This is only used
 for asymmetrical operations -, /, %, **.
 */
Node *    OperationBase::do_op_float_list(Node *node, const MincFloat val, const MincList *srcList, const OpKind op)
{
    ENTER();
    int i;
    MincValue *dest;
    if (srcList == NULL) {
        minc_warn("Null list in binary operation");
        node->setValue(MincValue((MincList *)NULL));
        return node;
    }
    const int len = srcList->len;
    MincValue *src = srcList->data;
    MincList *destList = new MincList(len);
    dest = destList->data;
    assert(len >= 0);
    switch (op) {
        case OpMinus:
            for (i = 0; i < len; i++) {
                if (src[i].dataType() == MincFloatType)
                    dest[i] = val - (MincFloat)src[i];
                else
                    dest[i] = src[i];
            }
            break;
        case OpDiv:
            for (i = 0; i < len; i++) {
                if (src[i].dataType() == MincFloatType)
                    dest[i] = val / (MincFloat)src[i];
                else
                    dest[i] = src[i];
            }
            break;
        case OpMod:
            for (i = 0; i < len; i++) {
                if (src[i].dataType() == MincFloatType)
                    dest[i] = (MincFloat) ((long) val % long((MincFloat)src[i]));
                else
                    dest[i] = src[i];
            }
            break;
        case OpPow:
            for (i = 0; i < len; i++) {
                if (src[i].dataType() == MincFloatType)
                    dest[i] = (MincFloat) pow((double) val, (MincFloat) src[i]);
                else
                    dest[i] = src[i];
            }
            break;
        default:
            for (i = 0; i < len; i++)
                dest[i] = 0.0;
            minc_internal_error("invalid float-list operator");
            break;
    }
    node->setValue(MincValue(destList));
    return node;
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

Node *	NodeLoadSym::doExct()
{
	/* Look up the symbol.  We check for success in finishExct(). */
	setSymbol(lookupSymbol(_symbolName, AnyLevel));
	return finishExct();
}

Node *	NodeLoadSym::finishExct()
{
    Symbol *nodeSymbol;
	if ((nodeSymbol = symbol()) != NULL) {
		TPRINT("%s: symbol %p\n", classname(), nodeSymbol);
        /* also assign the symbol's value into Node's value field */
        TPRINT("NodeLoadSym/NodeAutoDeclLoadSym: copying value from symbol '%s' to us\n", nodeSymbol->name());
        copyValue(nodeSymbol, true, true);
	}
	else {
        // Special trick: Store function name into Node's value
        TPRINT("NodeLoadSym: did not locate '%s' - storing name into Node in case it is a builtin function\n", symbolName());
        setValue(MincValue(symbolName()));
        // Now throw exception.  This will be caught in the case where a function call is being made.
        char msg[128];
		snprintf(msg, 128, "'%s' is not declared", symbolName());
        throw UndeclaredVariableException(msg);
	}
	return this;
}

Node *	NodeAutoDeclLoadSym::doExct()
{
    setValue(MincValue());  // reset value back to VoidType in case this Node is re-used in loop, etc.
	/* look up the symbol.  If we auto-declare it, use local scope for functions and if/while blocks */
    bool useLocalScope = inFunctionCall() || !inIfOrElseBlock();
	setSymbol(lookupOrAutodeclare(symbolName(), useLocalScope ? YES : NO));
	return finishExct();
}

Node *	NodeListElem::doExct()
{
	TPRINT("NodeListElem exct'ing Node link %p\n", child(0));
	child(0)->exct();
	if (sMincListLen == MAXDISPARGS) {
		minc_die("exceeded maximum number of items for a list");
		return this;
	}
	else {
		TPRINT("NodeListElem %p evaluating payload child Node %p\n", this, child(1));
		Node * tmp = child(1)->exct();
		/* Copy entire MincValue union from expr to this and to stack. */
		TPRINT("NodeListElem %p copying child value into self and to arguments MincList[%d]\n", this, sMincListLen);
        // We always allow overwrites of any list element with any other
		copyValue(tmp, /* allowTypeOverwrite=*/ true, /* suppressOverwriteWarning=*/ true);
		copyNodeToMincList(&sMincList[sMincListLen], tmp);
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
	// Copy from stack list into Node's MincList.
    for (int i = 0; i < sMincListLen; ++i) {
		theList->data[i] = sMincList[i];
    }
	pop_list();
	return this;
}

// The Subscript class is a private base class which allows several nodes to share its functionality

MincValue Subscript::readValueAtIndex(Node *listNode, Node *indexNode) {
    TPRINT("readValueAtIndex: Index via node %p (child 1)\n", indexNode);
    if (indexNode->dataType() != MincFloatType) {
        minc_die("list index must be a number");
    }
    MincFloat fltindex = (MincFloat) indexNode->value();
    int index = (int) fltindex;
    MincFloat frac = fltindex - index;
    MincList *theList = (MincList *) listNode->value();
    if (theList == NULL) {
        minc_die("attempt to index a NULL list");
    }
    int len = theList->len;
    if (len == 0) {
        minc_die("attempt to index an empty list");
    }
    if (fltindex < 0.0) {    /* -1 means last element */
        if (fltindex <= -2.0)
            minc_warn("negative index: returning last element");
        index = len - 1;
        frac = 0;
    } else if (fltindex > (MincFloat) (len - 1)) {
        minc_warn("attempt to index past the end of list '%s': returning last element", listNode->name());
        index = len - 1;
        frac = 0;
    }
    MincValue elem = theList->data[index];
    MincValue returnedValue = elem;
    /* do linear interpolation for float items */
    if (elem.dataType() == MincFloatType && frac > 0.0 && index < len - 1) {
        MincValue &elem2 = theList->data[index + 1];
        if (elem2.dataType() == MincFloatType) {
            returnedValue = (MincFloat) elem + (frac * ((MincFloat) elem2 - (MincFloat) elem));
        } else { /* can't interpolate btw. a number and another type */
            returnedValue = (MincFloat) elem;
        }
    }
    return returnedValue;
}

MincValue Subscript::searchWithMapKey(Node *mapNode, Node *key)
{
    ENTER();
    MincMap *theMap = (MincMap *) mapNode->symbol()->value();
    if (theMap == NULL) {
        minc_die("attempt to search a NULL map");
        return MincValue();     // void
    }
    const MincValue &valueIndex = key->value();
    std::map<MincValue, MincValue>::iterator it = theMap->map.find(valueIndex);
    if (it == theMap->map.end()) {
        minc_die("no item in map '%s' with that key", mapNode->symbol()->name());
        return MincValue();     // void
    }
    const MincValue &val = it->second;
    return val;
}


void Subscript::writeValueToIndex(Node *listNode, Node *indexNode, const MincValue &value)
{
    TPRINT("writeValueToIndex: Index via node %p\n", indexNode);
    if (indexNode->dataType() != MincFloatType) {
        minc_die("list index must be a number");
        return;
    }
    int len = 0;
    MincList *theList =  listNode->symbol() ? (MincList *) listNode->symbol()->value() : (MincList *) listNode->value();
    MincFloat fltindex = (MincFloat) indexNode->value();
    int index = (int) fltindex;
    if (fltindex - (MincFloat) index > 0.0)
        minc_warn("list index (%f) must be integer ... correcting", fltindex);
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
            Symbol *tsym = listNode->symbol();
            TPRINT("writeValueToIndex: setting new MincList on target %p's Symbol %p\n", listNode, tsym);
            theList = new MincList(len);
            tsym->setValue(MincValue(theList));
        } else {
            theList->resize(len);
        }
        TPRINT("writeValueToIndex: MincList %p expanded to len %d\n", theList->data, len);
    }
    theList->data[index] = value;
}

void    Subscript::writeWithMapKey(Node *mapNode, Node *indexNode, const MincValue &value)
{
    ENTER();
    // This is the 'store' operation for NodeSubscriptWrite, etc. - access the symbol's MincMap and update it.
    TPRINT("writeWithMapKey: Storing value into symbol's map\n");
    MincMap *theMap = (MincMap *) mapNode->symbol()->value();
    const MincValue &valueIndex = indexNode->value();
    if (theMap == NULL) {
        mapNode->symbol()->setValue(MincValue(theMap = new MincMap()));
    }
    theMap->map[valueIndex] = value;
}

Node *	NodeSubscriptRead::doExct()	// was exct_subscript_read()
{
	ENTER();
    TPRINT("NodeSubscriptRead: Object:\n");
	child(0)->exct();         /* lookup target */
    TPRINT("NodeSubscriptRead: Index:\n");
	child(1)->exct();         /* index */
    MincDataType child0Type = child(0)->dataType(); // This is the type of the object having operator [] applied.
    switch (child0Type) {
        case MincListType:
            setValue(readValueAtIndex(child(0), child(1)));
            break;
        case MincMapType:
            setValue(searchWithMapKey(child(0), child(1)));
            break;
        case MincStringType:
        {
            MincString theString = (MincString) child(0)->symbol()->value();
            MincFloat fltindex = (MincFloat) child(1)->value();
            int index = (int) fltindex;
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
                minc_warn("attempt to index past the end of string '%s': returning last element", child(0)->symbol()->name());
                index = stringLen - 1;
            }
            char stringChar[2];
            stringChar[1] = '\0';
            strncpy(stringChar, &theString[index], 1);
            MincValue elem((MincString)strsave(stringChar));  // create new string value from the one character
            this->setValue(elem);
        }
            break;
        default:
            minc_die("attempt to index or search an RHS-variable that's not a string, list, or map");
            break;
    }
	return this;
}

Node *	NodeSubscriptWrite::doExct()	// was exct_subscript_write()
{
	ENTER();
    TPRINT("NodeSubscriptWrite: Object exct:\n");
	child(0)->exct();         /* lookup target */
    TPRINT("NodeSubscriptWrite: Index exct:\n");
	child(1)->exct();         /* index */
    TPRINT("NodeSubscriptWrite: Exp to store exct:\n");
	child(2)->exct();         /* expression to store */
    Node *object = child(0);
    switch (object->dataType()) {
        case MincListType:
            writeValueToIndex(object, child(1), child(2)->value());
            break;
        case MincMapType:
            writeWithMapKey(object, child(1), child(2)->value());
            break;
        default:
            minc_die("attempt to index or store into L-variable '%s' which is not a list or map", object->name());
            break;
    }
    // Copying new variable types into a list is always OK
	copyValue(child(2), /* allowTypeOverwrite=*/ true, /* suppressOverwriteWarning=*/ true);
	return this;
}

Node * NodeSubscriptOpAssign::doExct()
{
    ENTER();
    TPRINT("NodeSubscriptOpAssign: Object:\n");
    child(0)->exct();         /* lookup target */
    TPRINT("NodeSubscriptOpAssign: Index:\n");
    child(1)->exct();         /* index */
    TPRINT("NodeSubscriptOpAssign: Exp to apply to element:\n");
    child(2)->exct();         /* expression to apply */
    MincDataType child0Type = child(0)->dataType(); // This is the type of the object having operator [] applied.
    switch (child0Type) {
        case MincListType:
            operateOnSubscript(child(0), child(1), child(2), op);
            break;
        case MincMapType:
            operateOnMapLookup(child(0), child(1), child(2), op);
            break;
        default:
            minc_die("attempt to operate on an L-variable that's not a list or map");
            break;
    }
    return this;
}

void NodeSubscriptOpAssign::operateOnSubscript(Node *listNode, Node *indexNode, Node *valueNode, OpKind op)
{
    ENTER();
    MincValue arrayValue = readValueAtIndex(listNode, indexNode);
    doOperation(this, arrayValue, valueNode->value(), op);
    writeValueToIndex(listNode, indexNode, this->value());
}

void NodeSubscriptOpAssign::operateOnMapLookup(Node *mapNode, Node *keyNode, Node *valueNode, OpKind op)
{
    ENTER();
    MincValue mapValue = searchWithMapKey(mapNode, keyNode);    // will throw if not found
    doOperation(this, mapValue, valueNode->value(), op);
    writeWithMapKey(mapNode, keyNode, this->value());
}

Node *  NodeMemberAccess::doExct()
{
    ENTER();
    TPRINT("NodeMemberAccess: get target object:\n");
    child(0)->exct();         /* lookup target */
    // NOTE: If LHS was a temporary variable, objectSymbol will be null
    Symbol *objectSymbol = child(0)->symbol();
    const char *targetName = (objectSymbol != NULL) ? objectSymbol->name() : "temp lhs";
    if (child(0)->dataType() == MincStructType) {
        TPRINT("NodeMemberAccess: access member '%s' on struct object '%s'\n", _memberName, targetName);
        MincStruct *theStruct = (MincStruct *) child(0)->value();
        if (theStruct) {
            Symbol *memberSymbol = theStruct->lookupMember(_memberName);
            if (memberSymbol) {
                // Member with this name was found
                setSymbol(memberSymbol);
                /* also assign the symbol's value into Node's value field */
                TPRINT("NodeMemberAccess: copying value from member symbol '%s' to us\n", _memberName);
                copyValue(memberSymbol);
            }
            else {
                minc_die("variable '%s' of type 'struct %s' has no member '%s'", targetName, theStruct->typeName(), _memberName);
            }
        }
        else {
            minc_die("struct variable '%s' is NULL", targetName);
        }
    }
    else {
        minc_die("variable '%s' is not a struct", targetName);
    }
    return this;
}

Node * MincFunctionHandler::callMincFunction(MincFunction *function, const char *functionName, Symbol *thisSymbol)
{
    Node *returnedNode = NULL;
    sCalledFunctions.push_back(functionName);
    assert(function != NULL);
    TPRINT("MincFunctionHandler::callMincFunction: theFunction = %p -- dropping in\n", function);
    push_function_stack();
    push_scope();           // move into function-body scope
    int savedLineNo=0, savedScope=0, savedCallDepth=0, savedIfElseDepth=0, savedForWhileDepth=0;
    try {
        // This replicates the argument-printing mechanism used by compiled-in functions.
        // Functions beginning with underbar, or those in a "suppressed list", can be "privatized" using set_option()
        if (RTOption::print() >= MMP_PRINTS) {
            const char *functionName = sCalledFunctions.back();
            bool isSuppressed = functionName[0] == '_' && RTOption::printSuppressUnderbar();
            if (!isSuppressed) {
                const char *suppressedList = RTOption::suppressedFunNamelist();
                if (suppressedList != NULL) {
                    char nameWithComma[128];    // there better not be a function name longer than this!
                    snprintf(nameWithComma, 128, "%s,", functionName);
                    isSuppressed = (strstr(suppressedList, nameWithComma) != NULL);
                }
            }
            if (!isSuppressed) {
                RTPrintf("============================\n");
                RTPrintfCat("%s: ", functionName);
                MincValue retval;
                call_builtin_function("print", sMincList, sMincListLen, &retval);
            }
        }
        // Create a symbol for 'this' within the function's scope if this is a method.
        function->handleThis(thisSymbol);
        /* The exp list is copied to the symbols for the function's arg list. */
        TPRINT("MincFunctionHandler::callMincFunction declaring all argument symbols in the function's scope\n");
        function->copyArguments();
        savedLineNo = yyget_lineno();
        savedScope = current_scope();
        savedIfElseDepth = sIfElseBlockDepth;
        savedForWhileDepth = sForWhileBlockDepth;
        sIfElseBlockDepth = 0;
        sForWhileBlockDepth = 0;
        incrementFunctionCallDepth();
        savedCallDepth = sFunctionCallDepth;
        TPRINT("MincFunctionHandler::callMincFunction executing %s(), call depth saved at %d\n",
               sCalledFunctions.back(), savedCallDepth);
        returnedNode = function->execute();
    }
    catch (Node * returned) {    // This catches return statements!
        TPRINT("MincFunctionHandler::callMincFunction caught Node %p as return stmt throw - restoring call depth %d\n",
               returned, savedCallDepth);
        returnedNode = returned;
        sFunctionCallDepth = savedCallDepth;
        sIfElseBlockDepth = savedIfElseDepth;
        sForWhileBlockDepth = savedForWhileDepth;
        SPRINT("Function call depth restored to %d, if/else depth to %d, for/while to %d",
               sFunctionCallDepth, sIfElseBlockDepth, sForWhileBlockDepth);
        restore_scope(savedScope);
    }
    catch(...) {    // Anything else is an error
        pop_function_stack();
        sCalledFunctions.pop_back();
        decrementFunctionCallDepth();
        throw;
    }
    decrementFunctionCallDepth();
    // restore parser line number
    yyset_lineno(savedLineNo);
    pop_function_stack();
    sCalledFunctions.pop_back();
    return returnedNode;
}

// In this special case, the function name is the struct type name

bool NodeFunctionCall::callConstructor(const char *functionName)
{
    TPRINT("NodeFunctionCall::callConstructor(%p) -- looking up struct type '%s'\n", this, functionName);
    const StructType *structType = lookupStructType(functionName, GlobalLevel);    // GlobalLevel for now
    if (structType) {
        TPRINT("NodeFunctionCall::callConstructor -- creating a value with type struct %s\n", functionName);
        // This replicates the argument-printing mechanism used by compiled-in functions.
        if (RTOption::print() >= MMP_PRINTS) {
            RTPrintf("============================\n");
            RTPrintfCat("%s: ", functionName);
            MincValue retval;
            call_builtin_function("print", sMincList, sMincListLen, &retval);
        }
        Symbol *sym = NULL;
        MincList *initList = NULL;
        try {
            // We create a temporary Symbol object in order to avoid moving or duplicating initAsStruct() method
            sym = Symbol::create("temporary");
            sym->_scope = 0;    // XXX
            // Wrap global sMincList to pass as argument
            initList = new MincList(0);
            initList->ref();
            initList->data = sMincList;
            initList->len = sMincListLen;
            TPRINT("NodeFunctionCall::callConstructor -- initializing struct members from sMincList\n");
            sym->initAsStruct(structType, initList);
            copyValue(sym, false);
            initList->data = NULL;
            initList->len = 0;
            initList->unref();
            initList = NULL;
            delete sym;
        } catch (...) {
            if (initList) {
                initList->data = NULL;
                initList->len = 0;
                initList->unref();
            }
            delete sym;
            throw;
        }
        return true;
    }
    return false;
}

void NodeFunctionCall::callBuiltinFunction(const char *functionName)
{
    if (!functionName) {
        minc_die("string variable called as function is NULL");
    }
    MincValue retval;
    int result = call_builtin_function(functionName, sMincList, sMincListLen,
                                       &retval);
    if (result == FUNCTION_NOT_FOUND) {
        result = call_external_function(functionName, sMincList, sMincListLen,
                                        &retval);
    }
    this->setValue(retval);
    switch (result) {
        case NO_ERROR:
            break;
        case FUNCTION_NOT_FOUND:
#if defined(ERROR_FAIL_ON_UNDEFINED_FUNCTION)
            throw result;
#endif
            break;
        default:
            throw result;
    }
}

Node *	NodeFunctionCall::doExct() {
    ENTER();
    Node *functionNode = child(0);
    Node *argList = child(1);
    try {
        TPRINT("NodeFunctionCall: Func: node %p (child 0)\n", functionNode);
        /* Lookup function.  NOTE: This operation can throw if the function is not a Minc function.
         * This is caught and handled below as a compiled function.
         */
        Node *calledFunction = functionNode->exct();
        push_list();
        TPRINT("NodeFunctionCall: Args: list node %p (child 1)\n", argList);
        argList->exct();    // execute arg expression list (stored on this NodeCall)
        switch (calledFunction->dataType()) {
            case MincFunctionType:        // Standalone MinC function
            {
                Symbol *functionSymbol = calledFunction->symbol();      // This can be NULL
                const char *functionName = functionSymbol ? functionSymbol->name() : "LHS";
                MincFunction *theFunction = (MincFunction *)calledFunction->value();
                if (theFunction != NULL) {
                    Node *returned = callMincFunction(theFunction, functionName);
                    if (returned != NULL) {
                        TPRINT("NodeFunctionCall copying Minc function call results into self\n");
                        copyValue(returned);
                    }
                }
                else {
                    minc_die("function variable '%s' is NULL", functionName);
                }
            }
                break;
            default:
                minc_die("%s'%s' is not a function or instrument", calledFunction->symbol() ? "variable " : "", calledFunction->symbol() ? calledFunction->symbol()->name() : "LHS");
                break;
        }
    } catch(UndeclaredVariableException &uve) {
        const char *builtinFunctionString = NULL;
        if (functionNode->dataType() == MincStringType) {
            // We stored this string away when we noticed this in the parser.
            builtinFunctionString = functionNode->value();
        }
        if (builtinFunctionString) {
            TPRINT("NodeFunctionCall: preparing for call to constructor or builtin function '%s'\n", builtinFunctionString);
            push_list();
            TPRINT("NodeFunctionCall: Args: list node %p (child 1)\n", argList);
            argList->exct();    // execute arg expression list (stored on this NodeCall)
            bool success = callConstructor((MincString) builtinFunctionString);
            if (!success) {
                TPRINT("NodeFunctionCall: no constructor - attempting call to builtin function '%s'\n", builtinFunctionString);
                callBuiltinFunction((MincString) builtinFunctionString);
            }
        }
        else {
            TPRINT("NodeFunctionCall: Other undeclared var exception - re-throwing\n");
            throw uve;
        }
    }
    pop_list();
    return this;
}

bool NodeMethodCall::callObjectMethod(Symbol *thisSymbol, const char *methodName) {
    MincValue thisValue = thisSymbol->value();
    TPRINT("NodeMethodCall::callObjectMethod: attempting to invoke '%s' on %s object\n", methodName, MincTypeName(thisValue.dataType()));
    MincValue retval;
    // Note: This function can modify 'thisValue' and/or return a value via 'retVal'
    int result = call_object_method(thisValue, methodName, sMincList, sMincListLen, &retval);
    if (result != 0) {
        thisSymbol->setValue(thisValue);
        this->setValue(retval);
        return true;        // success
    }
    return false;
}

Node *	NodeMethodCall::doExct()
{
    ENTER();
    TPRINT("NodeMethodCall: Object %p (child 0)\n", child(0));
    Node *object = child(0)->exct();
    TPRINT("NodeMethodCall: Method name: '%s'\n", _methodName);
    push_list();
    TPRINT("NodeMethodCall: Args: list node %p (child 1)\n", child(1));
    Node *args = child(1)->exct();    // execute arg expression list (stored on this NodeCall)
    if (object->dataType() == MincStructType) {
        // Method is being called on a struct object
        MincStruct *theStruct = (MincStruct *) object->value();
        if (theStruct) {
            // Check for "false method" - a mfunction struct member invoked as function.  No 'this' symbol.
            Symbol *memberSymbol = theStruct->lookupMember(_methodName);
            if (memberSymbol) {
                Node *functionRet;
                switch (memberSymbol->dataType()) {
                    case MincFunctionType:        // MinC function
                    {
                        MincFunction *theFunction = (MincFunction *)memberSymbol->value();
                        if (theFunction != NULL) {
                            functionRet = callMincFunction(theFunction, memberSymbol->name());
                            setValue(functionRet->value());     // store value from Minc function call to us
                        }
                        else {
                            minc_die("struct member '%s' is NULL", _methodName);
                        }
                    }
                        break;
                    default:
                        minc_die("struct member '%s' is not a function", _methodName);
                        break;
                }
            } else {
                // This is a "real method", so look for matching method symbol.
                TPRINT("NodeMethodCall: member with that name not found - attempting to retrieve symbol for method\n");
                const char *methodName = methodNameFromStructAndFunction(theStruct->typeName(), _methodName);
                Symbol *objectSymbol = object->symbol();
                Symbol *methodSymbol = lookupSymbol(methodName, AnyLevel);
                if (methodSymbol) {
                    MincFunction *theMethod = (MincFunction *)methodSymbol->value();
                    Node *functionRet = callMincFunction(theMethod, methodSymbol->name(), objectSymbol);
                    setValue(functionRet->value());     // store value from Minc method call to us
                } else if (objectSymbol) {
                    // See if method was one of the builtin object methods.
                    if (callObjectMethod(objectSymbol, _methodName) == false) {
                        minc_die("variable '%s' of type 'struct %s' has no member or method '%s'", object->name(),
                                 theStruct->typeName(), _methodName);
                    }
                }
                else {
                    // If the object's symbol was null, it was a LHS temp object
                    minc_die("Calling methods on temporary LHS objects is not supported");
                }
            }
        } else {
            minc_die("struct variable '%s' is NULL", object->name());       // XXX CHECK ME!
        }
    }
    else {
        // Method is being called on a non-struct object
        Symbol *objSymbol = object->symbol();
        if (objSymbol != NULL) {
            if (callObjectMethod(objSymbol, _methodName) == false) {   // Note: This stores the return value internally
                minc_die("Method '%s' is not defined for type '%s'", _methodName, MincTypeName(object->dataType()));
            }
        }
        else {
            minc_die("Calling methods on temporary LHS objects is not supported");
        }
    }
	pop_list();
    assert(this->dataType() != MincVoidType);
	return this;
}

Node *	NodeStore::doExct()
{
#ifdef ORIGINAL_CODE
	/* N.B. Now that symbol lookup is part of tree, this happens in
	 the NodeLoadSym stored as child[0] */
	TPRINT("NodeStore(%p): evaluate LHS %p (child 0)\n", this, child(0));
	child(0)->exct();
	/* evaluate RHS expression */
	TPRINT("NodeStore(%p): evaluate RHS (child 1)\n", this);
	child(1)->exct();
#else
	/* evaluate RHS expression */
	TPRINT("NodeStore(%p): evaluate RHS (child 1) FIRST\n", this);
	Node *rhs = child(1)->exct();
	/* N.B. Now that symbol lookup is part of tree, this happens in
	 the NodeLoadSym stored as child[0] */
	TPRINT("NodeStore(%p): evaluate LHS %p (child 0)\n", this, child(0));
	Node *lhs = child(0)->exct();
    // NEW: Do not allow overwrites of structs, functions, handles.
    MincDataType rhsType = rhs->dataType();
    MincDataType lhsType = lhs->dataType();
    if (lhsType != MincVoidType && lhsType != MincFloatType && lhsType != MincStringType) {
        _allowTypeOverwrite = false;
    }
#endif
	TPRINT("NodeStore(%p): copying value from RHS (%p) to LHS's symbol (%p)\n", this, rhs, lhs->symbol());
	/* Copy entire MincValue union from expr to id sym and to this. */
	lhs->symbol()->copyValue(child(1), _allowTypeOverwrite);
	TPRINT("NodeStore: copying value from RHS (%p) to here (%p)\n", rhs, this);
	copyValue(rhs, _allowTypeOverwrite);
	return this;
}

Node *	NodeOpAssign::doExct()		// was exct_opassign()
{
	ENTER();
	Node *tp0 = child(0)->exct();
	Node *tp1 = child(1)->exct();
    OpKind theOp = this->op;

    // ++ and -- are special-case for floats only
    if (theOp == OpPlusPlus || theOp == OpMinusMinus) {
        const char *opname = printOpKind(theOp);
        if (tp0->dataType() != MincFloatType || tp1->dataType() != MincFloatType) {
            minc_warn("can only use '%s' with number values", opname);
            copyValue(tp0->symbol());
            return this;
        }
        // doOperation does not support ++ and -- directly, so convert.
        theOp = (theOp == OpPlusPlus) ? OpPlus : OpMinus;
    }
	MincValue symValue = tp0->symbol()->value();
    MincValue rhs = tp1->value();
    doOperation(this, symValue, rhs, theOp);
    // N.B. doOperation() set the value on this node.  Now set it back on the symbol.
    tp0->symbol()->setValue(value());
	return this;
}

Node *	NodeNot::doExct()
{
	if ((bool)child(0)->exct()->value() == false)
		setValue(MincValue(1.0));
	else
        setValue(MincValue(0.0));
	return this;
}

Node *	NodeAnd::doExct()
{
    setValue(MincValue(0.0));
	if ((bool)child(0)->exct()->value() == true) {
		if ((bool)child(1)->exct()->value() == true) {
            setValue(MincValue(1.0));
		}
	}
	return this;
}

Node *	NodeRelation::doExct()		// was exct_relation()
{
	ENTER();
	const MincValue& v0 = child(0)->exct()->value();
	const MincValue& v1 = child(1)->exct()->value();
	
    try {
        MincFloat boolValue = 0.0;
        switch (this->op) {
            case OpEqual:
                boolValue = (v0 == v1) ? 1.0 : 0.0;
                break;
            case OpNotEqual:
                boolValue = (v0 != v1) ? 1.0 : 0.0;
                break;
            case OpLess:
                boolValue = (v0 < v1) ? 1.0 : 0.0;
                break;
            case OpGreater:
                boolValue = (v0 > v1) ? 1.0 : 0.0;
                break;
            case OpLessEqual:
                boolValue = (v0 <= v1) ? 1.0 : 0.0;
                break;
            case OpGreaterEqual:
                boolValue = (v0 >= v1) ? 1.0 : 0.0;
                break;
            default:
                minc_internal_error("exct: tried to execute invalid NodeRelation");
                break;
        }
        setValue(MincValue(boolValue));
    }
    catch (NonmatchingTypeException &e) {
        minc_warn("operator '%s': attempt to compare variables having different types - returning false", printOpKind(this->op));
        setValue(MincValue(0.0));
    }
    catch (InvalidTypeException &e) {
        minc_warn("operator '%s': cannot compare variables of this type - returning false", printOpKind(this->op));
        setValue(MincValue(0.0));
    }
	return this;
}

OperationBase::OperationBase() {}

Node *OperationBase::doOperation(Node *node, const MincValue &lhs, const MincValue &rhs, OpKind op)
{
    switch (lhs.dataType()) {
        case MincFloatType:
            switch (rhs.dataType()) {
                case MincFloatType:
                    do_op_num(node, (MincFloat)lhs, (MincFloat)rhs, op);
                    break;
                case MincStringType:
                    if (op == OpPlus) {
                        char buf[64];
                        snprintf(buf, 64, "%g", (MincFloat) lhs);
                        do_op_string(node, (MincString) buf, (MincString) rhs, op);
                    }
                    else {
                        minc_warn("operator '%s': invalid operation for a float and a string", printOpKind(op));
                    }
                    break;
                case MincHandleType:
                    do_op_num_handle(node, (MincFloat)lhs, (MincHandle)rhs, op);
                    break;
                case MincListType:
                    /* Check for asymmetrical ops. */
                    switch (op) {
                        case OpMinus:
                        case OpDiv:
                        case OpMod:
                        case OpPow:
                            do_op_float_list(node, (MincFloat)lhs, (MincList*)rhs, op);
                            break;
                        default:
                            do_op_list_float(node, (MincList*)rhs, (MincFloat)lhs, op);
                            break;
                    }
                    break;
                case MincMapType:
                case MincStructType:
                case MincFunctionType:
                    minc_warn("operator '%s': can't operate on a float and a %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
                default:
                    minc_internal_error("operator '%s': invalid rhs type: %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
            }
            break;
        case MincStringType:
            switch (rhs.dataType()) {
                case MincFloatType:
                    // "do_op_string_float"
                    if (op == OpPlus) {
                        char buf[64];
                        snprintf(buf, 64, "%g", (MincFloat)rhs);
                        do_op_string(node, (MincString)lhs, buf, op);
                    }
                    else {
                        minc_warn("operator '%s': invalid operation for a string and a float", printOpKind(op));
                    }
                    break;
                case MincStringType:
                    do_op_string(node, (MincString)lhs, (MincString)rhs, op);
                    break;
                case MincHandleType:
                case MincListType:
                case MincMapType:
                case MincStructType:
                case MincFunctionType:
                    minc_warn("operator '%s': can't operate on a string and a %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
                default:
                    minc_internal_error("operator '%s': invalid rhs type: %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
            }
            break;
        case MincHandleType:
            switch (rhs.dataType()) {
                case MincFloatType:
                    do_op_handle_num(node, (MincHandle)lhs, (MincFloat)rhs, op);
                    break;
                case MincStringType:
                    minc_warn("operator '%s': can't operate on a string and a handle", printOpKind(op));
                    break;
                case MincHandleType:
                    do_op_handle_handle(node, (MincHandle)lhs, (MincHandle)rhs, op);
                    break;
                case MincListType:
                case MincMapType:
                case MincStructType:
                case MincFunctionType:
                    minc_warn("operator '%s': can't operate on a handle and a %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
                default:
                    minc_internal_error("operator '%s': invalid rhs type: %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
            }
            break;
        case MincListType:
            switch (rhs.dataType()) {
                case MincFloatType:
                    do_op_list_float(node, (MincList *)lhs, (MincFloat)rhs, op);
                    break;
                case MincStringType:
                    minc_warn("operator '%s': can't operate on a list and a string", printOpKind(op));
                    break;
                case MincHandleType:
                    minc_warn("operator '%s': can't operate on a list and a handle", printOpKind(op));
                    break;
                case MincListType:
                    do_op_list_list(node, (MincList *)lhs, (MincList *)rhs, op);
                    break;
                case MincMapType:
                case MincStructType:
                case MincFunctionType:
                    minc_warn("operator '%s': can't operate on a list and a %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
                default:
                    minc_internal_error("operator '%s': invalid rhs type: %s", printOpKind(op), MincTypeName(rhs.dataType()));
                    break;
            }
            break;
        case MincStructType:
        case MincMapType:
        case MincFunctionType:
            minc_warn("operator '%s': a %s cannot be used for this operation", printOpKind(op), MincTypeName(lhs.dataType()));
            break;
        default:
            minc_internal_error("operator '%s': invalid lhs type: %s", printOpKind(op), MincTypeName(lhs.dataType()));
            break;
    }
    return node;
}

Node *	NodeOp::doExct()
{
	ENTER();
	const MincValue& v0 = child(0)->exct()->value();
	const MincValue& v1 = child(1)->exct()->value();
    doOperation(this, v0, v1, this->op);
	return this;
}

Node *	NodeUnaryOperator::doExct()
{
	if (this->op == OpNeg)
		setValue(MincValue(-1 * (MincFloat)child(0)->exct()->value()));
	return this;
}

Node *	NodeOr::doExct()
{
	setValue(MincValue(0.0));
	if (((bool)child(0)->exct()->value() == true) ||
		((bool)child(1)->exct()->value() == true)) {
        setValue(MincValue(1.0));
	}
	return this;
}

// Note: For backwards-compat, if() blocks need to be global scope *even if* contained within
// a for/while block.  Hence the clearing and restoring of the block depth count here and in the next class.

Node *	NodeIf::doExct()
{
    incrementIfElseBlockDepth();
    if ((bool)child(0)->exct()->value() == true) {
		child(1)->exct();
    }
    decrementIfElseBlockDepth();
	return this;
}

Node *	NodeIfElse::doExct()
{
    incrementIfElseBlockDepth();
    if ((bool)child(0)->exct()->value() == true) {
        child(1)->exct();
    }
	else {
        child(2)->exct();
    }
    decrementIfElseBlockDepth();
	return this;
}

// Note: while() blocks use local scope, which is controlled by the block depth counter

Node *	NodeWhile::doExct()
{
    incrementForWhileBlockDepth();
    while ((bool)child(0)->exct()->value() == true) {
		child(1)->exct();
    }
    decrementForWhileBlockDepth();
	return this;
}

Node *	NodeArgList::doExct()
{
	sArgListLen = 0;
	sArgListIndex = 0;	// reset to walk list
	inCalledFunctionArgList = true;
	TPRINT("NodeArgList: walking function '%s()' arg decl/copy list\n", sCalledFunctions.back());
	child(0)->exct();
	inCalledFunctionArgList = false;
    // Create a special function block symbol storing the function's argument count.
    Symbol *n_args = installSymbol(strsave("_n_args"), NO);
    n_args->setValue(MincValue((MincFloat) sArgListLen));
	return this;
}

Node *	NodeArgListElem::doExct()
{
	++sArgListLen;
	child(0)->exct();	// work our way to the front of the list
    TPRINT("NodeArgListElem(%p): execute arg decl %p (child 1)\n", this, child(1));
	child(1)->exct();	// run the arg decl
	// Symbol associated with this function argument
	Symbol *argSym = child(1)->symbol();
	if (sMincListLen > sArgListLen) {
		minc_die("%s() takes %d arguments but was passed %d!", sCalledFunctions.back(), sArgListLen, sMincListLen);
	}
	else if (sArgListIndex >= sMincListLen) {
        if (sMincWarningLevel > MincNoDefaultedArgWarnings) {
            minc_warn("%s(): arg %d ('%s') not provided - defaulting to 0", sCalledFunctions.back(), sArgListIndex, argSym->name());
        }
		/* Copy zeroed MincValue to us and then to sym. */
		MincValue zeroElem;
		zeroElem = argSym->value();	// this captures the data type
		zeroElem.zero();
		this->setValue(zeroElem);
		argSym->copyValue(this);
		++sArgListIndex;
	}
	/* compare stored NodeLoadSym with user-passed arg */
	else {
        TPRINT("NodeArgListElem: verify argument %d type and initialize with passed-in sMincList[%d] value\n", sArgListIndex, sArgListIndex);
		// Pre-cached argument value from caller
		MincValue &argValue = sMincList[sArgListIndex];
		bool compatible = false;
		switch (argValue.dataType()) {
			case MincFloatType:
			case MincStringType:
			case MincHandleType:
			case MincListType:
            case MincMapType:
            case MincStructType:
            case MincFunctionType:
            case MincVoidType:
				if (argSym->dataType() != argValue.dataType()) {
					minc_die("%s() arg %d ('%s') passed as %s, expecting %s",
								sCalledFunctions.back(), sArgListIndex, argSym->name(), MincTypeName(argValue.dataType()), MincTypeName(argSym->dataType()));
				}
				else compatible = true;
				break;
			default:
                minc_internal_error("%s() arg %d ('%s') is an unhandled type!", sCalledFunctions.back(), sArgListIndex, argSym->name());
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
    TPRINT("NodeRet(%p): Evaluate returned value %p (child 0)\n", this, child(0));
	child(0)->exct();
	copyValue(child(0));
	TPRINT("NodeRet throwing %p for return stmt\n", this);
	throw this;	// Cool, huh?  Throws this node's body out to function's endpoint!
	return NULL;	// notreached
}

Node *	NodeFuncBodySeq::doExct()
{
    TPRINT("NodeFuncBodySeq(%p): Executing function body\n", this);
	child(0)->exct();
    TPRINT("NodeFuncBodySeq executing return statement\n");
	child(1)->exct();
	copyValue(child(1));
	return this;
}

// Note: for() blocks use local scope, which is controlled by the block depth counter

Node *	NodeFor::doExct()
{
	child(0)->exct();         /* init */
    incrementForWhileBlockDepth();
	while ((bool)child(1)->exct()->value() == true) { /* condition */
		_child4->exct();      /* execute block */
		child(2)->exct();      /* prepare for next iteration */
	}
    decrementForWhileBlockDepth();
	return this;
}

Node *	NodeSeq::doExct()
{
	child(0)->exct();
	child(1)->exct();
	return this;
}

// Note: Block scopes behave differently when we are in an if/else block for backwards compatibility.  However,
// if we are inside a Minc function call block, all backwards-compat is turned off.

Node *	NodeBlock::doExct()
{
    if (!inIfOrElseBlock() || inFunctionCall()) {
        push_scope();
    }
	child(0)->exct();
    if (!inIfOrElseBlock() || inFunctionCall()) {
        pop_scope();
    }
	return this;				// NodeBlock returns void type
}

Node *	NodeDecl::doExct()
{
	TPRINT("NodeDecl(%p) -- declaring variable '%s'\n", this, _symbolName);
	Symbol *sym = lookupSymbol(_symbolName, inCalledFunctionArgList ? ThisLevel : AnyLevel);
	if (!sym) {
		sym = installSymbol(_symbolName, NO);
		sym->setValue(MincValue(this->_type));
	}
	else {
		if (sym->scope() == current_scope()) {
			if (inCalledFunctionArgList) {
				minc_die("%s(): argument variable '%s' already used", sCalledFunctions.back(), _symbolName);
			}
			minc_warn("variable '%s' redefined - using existing one", _symbolName);
		}
		else {
			if (!inFunctionCall() && !inCalledFunctionArgList) {
				minc_warn("variable '%s' also defined at enclosing scope", _symbolName);
			}
			sym = installSymbol(_symbolName, NO);
			sym->setValue(MincValue(this->_type));
		}
	}
	this->setSymbol(sym);
	return this;
}

// NodeStructDef stores the new struct type into the scope's struct type table.
// sNewStructType is used by NodeMemberDecl

Node *  NodeStructDef::doExct()
{
    TPRINT("NodeStructDef(%p) -- installing struct type '%s'\n", this, _typeName);
    if (current_scope() == 0) {    // until I allow nested structs
        sNewStructType = installStructType(_typeName, YES);  // all structs global for now
        if (sNewStructType) {
            TPRINT("-- walking struct's element list\n");
            child(0)->exct();
            sNewStructType = NULL;
        }
    }
    else {
        minc_die("struct definitions only allowed in global scope for now");
    }
    return this;
}

// NodeMemberDecl is invoked once for each struct member listed in the struct definition.  sNewStructType was set in NodeStructDef.

Node *  NodeMemberDecl::doExct()
{
    TPRINT("NodeMemberDecl(%p) -- storing decl info for member '%s', type %s\n", this, _symbolName, MincTypeName(this->_type));
    assert(sNewStructType != NULL);
    sNewStructType->addMemberInfo(_symbolName, this->_type, this->_symbolSubtype);
    return this;
}

// NodeStructDecl does the actual work of creating an instance of a particular struct type

Node *    NodeStructDecl::doExct()
{
    TPRINT("NodeStructDecl(%p) -- looking up struct type '%s'\n", this, _typeName);
    const StructType *structType = lookupStructType(_typeName, GlobalLevel);    // GlobalLevel for now
    if (structType) {
        TPRINT("-- declaring variable '%s', type struct %s\n", _symbolName, _typeName);
        Node *initializers = child(0);
        if (initializers) {
            initializers->exct();   // This can throw
        }
        // DAS changed this 7/18/22 - was brokenly only checking at global scope
        Symbol *sym = lookupSymbol(_symbolName, AnyLevel);
        if (!sym) {
            sym = installSymbol(_symbolName, NO);   // install sym at current scope
            MincList *initList = (initializers) ? (MincList *)initializers->value() : NULL;
            sym->initAsStruct(structType, initList);
        }
        else {
            if (sym->scope() == current_scope()) {
                if (inCalledFunctionArgList) {
                    minc_die("%s(): argument variable '%s' already used", sCalledFunctions.back(), _symbolName);
                }
                else if (initializers != NULL) {
                    minc_die("cannot redefine struct variable '%s' with initializers", _symbolName);
                }
                minc_warn("variable '%s' redefined - using existing one", _symbolName);
            }
            else {
                if (!inFunctionCall() && !inCalledFunctionArgList) {
                    minc_warn("variable '%s' also defined at enclosing scope", _symbolName);
                }
                sym = installSymbol(_symbolName, NO);
                MincList *initList = (initializers) ? (MincList *)initializers->value() : NULL;
                sym->initAsStruct(structType, initList);
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
	TPRINT("NodeFuncDecl(%p) -- declaring function '%s'\n", this, _symbolName);
    if (current_scope() > 0) {
        minc_die("functions may only be declared at global scope");
    }
	Symbol *sym = lookupSymbol(_symbolName, GlobalLevel);	// only look at current global level
	if (sym == NULL) {
		sym = installSymbol(_symbolName, YES);		// all functions global for now
		sym->setValue(MincValue(MincFunctionType));      // Empty MincFunction value
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

Node *    NodeMethodDecl::doExct()
{
    TPRINT("NodeMethodDecl(%p) -- declaring method '%s'\n", this, _symbolName);
    if (current_scope() > 0) {
        minc_die("methods may only be declared at global scope");
    }
    const char *mangledName = methodNameFromStructAndFunction(_structTypeName, _symbolName);
    Symbol *sym = lookupSymbol(mangledName, GlobalLevel);    // only look at current global level
    if (sym == NULL) {
        sym = installSymbol(mangledName, YES);        // all functions global for now
        sym->setValue(MincValue(MincFunctionType));      // Empty MincFunction value
        this->setSymbol(sym);
    }
    else {
#ifdef EMBEDDED
        minc_warn("method %s() is already declared", _symbolName);
        this->setSymbol(sym);
#else
        minc_die("method %s() is already declared for struct %s", _symbolName, _structTypeName);
#endif
    }
    return this;
}

Node *	NodeFuncDef::doExct()
{
	// Look up symbol for function, and bind the function's "guts" to it via a MincFunction.
	TPRINT(" (%p): executing lookup/install node %p (child 0)\n", this, child(0));
	child(0)->exct();
	assert(child(0)->symbol() != NULL);
    // Note: arglist and body stored inside MincFunction.  This is how we store the behavior
    // of a function/method on its symbol for re-use.  Creating with MincFunction::Method causes it to
    // expect to find a symbol for 'this'.
	child(0)->symbol()->setValue(MincValue(new MincFunction(child(1), child(2), _isMethod ? MincFunction::Method : MincFunction::Standalone)));
	return this;
}

static void
push_list()
{
	ENTER();
    if (list_stack_ptr >= MAXSTACK) {
      minc_die("stack overflow: too many nested list levels or function calls");
    }
   list_stack[list_stack_ptr] = sMincList;
   list_len_stack[list_stack_ptr++] = sMincListLen;
   sMincList = new MincValue[MAXDISPARGS];
   TPRINT("push_list: sMincList=%p at new stack level %d, len %d\n", sMincList, list_stack_ptr, sMincListLen);
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
copyNodeToMincList(MincValue *dest, Node *tpsrc)
{
   TPRINT("copyNodeToMincList(%p, %p)\n", dest, tpsrc);
#ifdef EMBEDDED
	/* Not yet handling nonfatal errors with throw/catch */
	if (tpsrc->dataType() == MincVoidType) {
		return;
	}
#endif
	*dest = tpsrc->value();
}
