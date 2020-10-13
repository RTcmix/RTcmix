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
*/

/* This file holds the intermediate tree representation. */

#define DEBUG

#include "debug.h"

#include "Node.h"
#include "MincValue.h"
#include "Scope.h"
#include "Symbol.h"
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

/* builtin.cpp */
extern int call_builtin_function(const char *funcname, const MincValue arglist[],
                          const int nargs, MincValue *retval);

/* callextfunc.cpp */
extern int call_external_function(const char *funcname, const MincValue arglist[],
                           const int nargs, MincValue *return_value);
extern MincHandle minc_binop_handle_float(const MincHandle handle, const MincFloat val, OpKind op);
extern MincHandle minc_binop_float_handle(const MincFloat val, const MincHandle handle, OpKind op);
extern MincHandle minc_binop_handles(const MincHandle handle1, const MincHandle handle2, OpKind op);

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
static std::vector<const char *> sCalledFunctions;
static int sFunctionCallDepth = 0;	// level of actively-executing function calls

static bool inFunctionCall() { return sFunctionCallDepth > 0; }

static void copy_tree_listelem(MincValue *edest, Node *  tpsrc);
static void print_symbol(Symbol * s);		// TODO: Symbol::print()

static MincWarningLevel sMincWarningLevel = MincAllWarnings;

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
	sCalledFunctions.clear();
	sFunctionCallDepth = 0;
	sArgListLen = 0;
	sArgListIndex = 0;
}

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
   "NodeLoadSym",
   "NodeAutoDeclLoadSym",
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
static void push_list(void);
static void pop_list(void);

/* ========================================================================== */
/* Tree nodes */

Node::Node(OpKind op, NodeKind kind)
	: kind(kind), op(op), lineno(yyget_lineno())
{
	TPRINT("Node::Node (%s) this=%p\n", classname(), this);
    u.number = 0.0;     // this should zero out the union.
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

const char * Node::name() const
{
    return (u.symbol) ? u.symbol->name() : "UNDEFINED";
}

const char * Node::classname() const
{
	return printNodeKind(kind);
}

void Node::print()
{
	TPRINT("Node %p: %s type: %d\n", this, classname(), this->dataType());
	if (kind == eNodeLoadSym) {
		TPRINT("Symbol:\n");
		print_symbol(u.symbol);
	}
	else if (this->dataType() == MincVoidType && child(0) != NULL) {
		TPRINT("Child 0:\n");
		child(0)->print();
	}
    else {
        TPRINT("value: ");
        value().print();
    }
}

Node *	Node::exct()
{
	ENTER();
	TPRINT("%s::exct() this=%p\n", classname(), this);
	if (inFunctionCall() && lineno > 0) {
		yyset_lineno(lineno);
	}
	Node *outNode = doExct();	// this is redefined on all subclasses
    TPRINT("%s::exct() done: returning node %p of type %s\n", classname(), outNode, MincTypeName(outNode->dataType()));
	return outNode;
}

/* This copies a node's value and handles ref counting when necessary */
Node *
Node::copyValue(Node *source)
{
    TPRINT("Node::copyValue(this=%p, Node=%p)\n", this, source);
#ifdef EMBEDDED
    /* Not yet handling nonfatal errors with throw/catch */
    if (source->dataType() == MincVoidType) {
        return this;
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
    TPRINT("Node::copyValue(this=%p, Symbol=%p)\n", this, src);
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
        minc_warn("invalid operator for two strings");
        this->v = (char *)NULL;
        break;
      case OpNeg:
		minc_warn("invalid operator on string");
        break;
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
		minc_internal_error("invalid binary handle operator");
		break;
	}
	return this;
}

/* ---------------------------------------------------- do_op_list_float -- */
/* Iterate over the list, performing the operation specified by <op>,
   using the scalar <val>, for each list element - with the element first in the equation.  Store the result into a
   new list for <this>, so that child's list is unchanged.
*/
Node *	NodeOp::do_op_list_float(const MincList *srcList, const MincFloat val, const OpKind op)
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

/* ---------------------------------------------------- do_op_float_list -- */
/* Iterate over the list, performing the operation specified by <op>,
 using the scalar <val>, for each list element - with <val> first in the equation.  Store the result into a
 new list for <this>, so that child's list is unchanged.  NOTE:  This is only used
 for asymmetrical operations -, /, %, **.
 */
Node *    NodeOp::do_op_float_list(const MincFloat val, const MincList *srcList, const OpKind op)
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
    this->value() = destList;
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

Node *	NodeLoadSym::doExct()
{
	/* look up the symbol */
	setSymbol(lookupSymbol(_symbolName, AnyLevel));
	return finishExct();
}

Node *	NodeLoadSym::finishExct()
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
			TPRINT("NodeLoadSym/NodeAutoDeclLoadSym: copying value from symbol '%s' to us\n", nodeSymbol->name());
			copyValue(nodeSymbol);
		}
	}
	else {
		minc_die("'%s' is not declared", symbolName());
	}
	return this;
}

Node *	NodeAutoDeclLoadSym::doExct()
{
	/* look up the symbol */
	setSymbol(lookupOrAutodeclare(symbolName(), inFunctionCall() ? YES : NO));
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

void    NodeSubscriptRead::readAtSubscript()
{
    ENTER();
    TPRINT("NodeSubscriptRead(%p): Index via node %p (child 1)\n", this, child(1));
    if (child(1)->dataType() != MincFloatType) {
        minc_die("list index must be a number");
        return;
    }
    MincFloat fltindex = (MincFloat) child(1)->value();
    int index = (int) fltindex;
    MincFloat frac = fltindex - index;
//    MincList *theList = (MincList *) child(0)->symbol()->value();
    MincList *theList = (MincList *) child(0)->value();
    if (theList == NULL) {
        minc_die("attempt to index a NULL list");
        return;
    }
    int len = theList->len;
    if (len == 0) {
        minc_die("attempt to index an empty list");
        return;
    }
    if (fltindex < 0.0) {    /* -1 means last element */
        if (fltindex <= -2.0)
            minc_warn("negative index: returning last element");
        index = len - 1;
        frac = 0;
    }
    else if (fltindex > (MincFloat) (len - 1)) {
        minc_warn("attempt to index past the end of list '%s': returning last element", child(0)->symbol()->name());
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

void    NodeSubscriptRead::searchWithMapKey()
{
    MincMap *theMap = (MincMap *) child(0)->symbol()->value();
    if (theMap == NULL) {
        minc_die("attempt to search a NULL map");
        return;
    }
    const MincValue &valueIndex = child(1)->value();
    std::map<MincValue, MincValue>::iterator it = theMap->map.find(valueIndex);
    if (it == theMap->map.end()) {
        minc_die("no item in map '%s' with that key", child(0)->symbol()->name());
        return;
    }
    const MincValue &val = it->second;
    this->setValue(val);
}

Node *	NodeSubscriptRead::doExct()	// was exct_subscript_read()
{
	ENTER();
    TPRINT("Object:\n");
	child(0)->exct();         /* lookup target */
    TPRINT("Index:\n");
	child(1)->exct();         /* index */
//    MincDataType child0Type = child(0)->symbol()->dataType();
    MincDataType child0Type = child(0)->dataType();
    switch (child0Type) {
        case MincListType:
            readAtSubscript();
            break;
        case MincMapType:
            searchWithMapKey();
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
            MincValue elem((MincString)strdup(stringChar));  // create new string value from the one character
            this->setValue(elem);
        }
            break;
        default:
            minc_die("attempt to index or search an RHS-variable that's not a string, list, or map");
            break;
    }
	return this;
}

void    NodeSubscriptWrite::writeToSubscript()
{
    ENTER();
    if (child(1)->dataType() != MincFloatType) {
        minc_die("list index must be a number");
        return;
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
}

void    NodeSubscriptWrite::writeWithMapKey()
{
    MincMap *theMap = (MincMap *) child(0)->symbol()->value();
    const MincValue &valueIndex = child(1)->value();
    if (theMap == NULL) {
        child(0)->symbol()->value() = theMap = new MincMap();
    }
    theMap->map[valueIndex] = child(2)->value();
}

Node *	NodeSubscriptWrite::doExct()	// was exct_subscript_write()
{
	ENTER();
    TPRINT("Object:\n");
	child(0)->exct();         /* lookup target */
    TPRINT("Index:\n");
	child(1)->exct();         /* index */
    TPRINT("Exp to store:\n");
	child(2)->exct();         /* expression to store */
    switch (child(0)->symbol()->dataType()) {
        case MincListType:
            writeToSubscript();
            break;
        case MincMapType:
            writeWithMapKey();
            break;
        default:
            minc_die("attempt to index or store into an L-variable that's not a list or map");
            break;
    }
	copyValue(child(2));
	return this;
}

Node *  NodeMember::doExct()
{
    ENTER();
    TPRINT("Object:\n");
    child(0)->exct();         /* lookup target */
    // NOTE: If LHS was a temporary variable, structSymbol will be null
    Symbol *structSymbol = child(0)->symbol();
    const char *targetName = (structSymbol != NULL) ? structSymbol->name() : "lhs-variable";
    if (child(0)->dataType() == MincStructType) {
       Symbol *memberSymbol = ((MincStruct *) child(0)->value())->lookupMember(_memberName);
       if (memberSymbol) {
            setSymbol(memberSymbol);
            /* also assign the symbol's value into tree's value field */
            TPRINT("NodeMember: copying value from member symbol '%s' to us\n", _memberName);
            copyValue(memberSymbol);
        }
        else {
            minc_die("struct variable '%s' has no member '%s'", targetName, _memberName);
        }
    }
    else {
        minc_die("variable '%s' is not a struct", targetName);
    }
    return this;
}

Node *	NodeCall::doExct()
{
    ENTER();
	push_list();
	Symbol *funcSymbol = lookupSymbol(_functionName, GlobalLevel);
	if (funcSymbol) {
		sCalledFunctions.push_back(_functionName);
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
			int savedLineNo=0, savedScope=0, savedCallDepth=0;
			Node * temp = NULL;
			try {
                // This replicates the argument-printing mechanism used by compiled-in functions.
                if (Option::print() >= MMP_PRINTS) {
                    RTPrintf("============================\n");
                    RTPrintfCat("%s: ", sCalledFunctions.back());
                    MincValue retval;
                    call_builtin_function("print", sMincList, sMincListLen, &retval);
                }
				/* The exp list is copied to the symbols for the function's arg list. */
				funcDef->child(1)->exct();
				savedLineNo = yyget_lineno();
				savedScope = current_scope();
				++sFunctionCallDepth;
				savedCallDepth = sFunctionCallDepth;
				TPRINT("NodeCall(%p): executing %s() block node %p, call depth now %d\n",
					   this, sCalledFunctions.back(), funcDef->child(2), savedCallDepth);
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
		sCalledFunctions.pop_back();
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
#if defined(ERROR_FAIL_ON_UNDEFINED_FUNCTION)
                throw result;
#endif
                break;
            default:
                throw result;
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
	 the NodeLoadSym stored as child[0] */
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
	 the NodeLoadSym stored as child[0] */
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
        if (op == OpPlusPlus) {
            minc_warn("can only use '++' with numbers");
        }
        else if (op == OpMinusMinus) {
            minc_warn("can only use '--' with numbers");
        }
        else {
            minc_warn("can only use '%c=' with numbers",
                      op == OpPlus ? '+' : (op == OpMinus ? '-'
                                            : (op == OpMul ? '*' : '/')));
        }
		copyValue(tp0->symbol());
		return this;
	}
	MincValue& symValue = tp0->symbol()->value();
	MincFloat rhs = (MincFloat)tp1->value();
	switch (this->op) {
		case OpPlus:
        case OpPlusPlus:
			symValue = (MincFloat)symValue + rhs;
			break;
		case OpMinus:
        case OpMinusMinus:
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
	if ((bool)child(0)->exct()->value() == false)
		this->value() = 1.0;
	else
		this->value() = 0.0;
	return this;
}

Node *	NodeAnd::doExct()
{
	this->value() = 0.0;
	if ((bool)child(0)->exct()->value() == true) {
		if ((bool)child(1)->exct()->value() == true) {
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
		minc_warn("operator %s: attempt to compare variables having different types - returning false", printOpKind(this->op));
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
    minc_warn("operator %s: cannot compare variables of this type - returning false", printOpKind(this->op));
    this->value() = 0.0;
    return this;
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
					/* Check for asymmetrical ops. */
                    switch (this->op) {
                        case OpMinus:
                        case OpDiv:
                        case OpMod:
                        case OpPow:
                            do_op_float_list((MincFloat)v0, (MincList*)v1, this->op);
                            break;
                        default:
                            do_op_list_float((MincList*)v1, (MincFloat)v0, this->op);
                            break;
                    }
					break;
                case MincMapType:
                    minc_warn("operator %s: a map cannot be used for this operation", printOpKind(this->op));
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
                    // "do_op_string_float"
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
                case MincMapType:
                    minc_warn("can't operate on a string and a map");
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
					minc_warn("operator %s: can't operate on a string and a handle", printOpKind(this->op));
					break;
				case MincHandleType:
					do_op_handle_handle((MincHandle)v0, (MincHandle)v1, this->op);
					break;
				case MincListType:
					minc_warn("operator %s: can't operate on a list and a handle", printOpKind(this->op));
					break;
                case MincMapType:
                    minc_warn("operator %s: a map cannot be used for this operation", printOpKind(this->op));
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
					do_op_list_float((MincList *)v0, (MincFloat)v1, this->op);
					break;
				case MincStringType:
					minc_warn("operator %s: can't operate on a list and a string", printOpKind(this->op));
					break;
				case MincHandleType:
					minc_warn("operator %s: can't operate on a list and a handle", printOpKind(this->op));
					break;
				case MincListType:
					do_op_list_list((MincList *)v0, (MincList *)v1, this->op);
					break;
                case MincMapType:
                    minc_warn("operator %s: a map cannot be used for this operation", printOpKind(this->op));
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
	if (((bool)child(0)->exct()->value() == true) ||
		((bool)child(1)->exct()->value() == true)) {
		this->value() = 1.0;
	}
	return this;
}

Node *	NodeIf::doExct()
{
    if ((bool)child(0)->exct()->value() == true)
		child(1)->exct();
	return this;
}

Node *	NodeIfElse::doExct()
{
    if ((bool)child(0)->exct()->value() == true)
		child(1)->exct();
	else
		child(2)->exct();
	return this;
}

Node *	NodeWhile::doExct()
{
    while ((bool)child(0)->exct()->value() == true) {
		child(1)->exct();
    }
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
	return this;
}

Node *	NodeArgListElem::doExct()
{
	++sArgListLen;
	child(0)->exct();	// work our way to the front of the list
    TPRINT("NodeArgListElem(%p): run arg decls %p (child 1)\n", this, child(1));
	child(1)->exct();	// run the arg decl
	// Symbol associated with this function argument
	Symbol *argSym = child(1)->symbol();
	if (sMincListLen > sArgListLen) {
		minc_die("%s() takes %d arguments but was passed %d!", sCalledFunctions.back(), sArgListLen, sMincListLen);
	}
	else if (sArgListIndex >= sMincListLen) {
        if (sMincWarningLevel > MincNoDefaultedArgWarnings) {
            minc_warn("%s(): arg '%s' not provided - defaulting to 0", sCalledFunctions.back(), argSym->name());
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
				if (argSym->dataType() != argValue.dataType()) {
					minc_die("%s() arg '%s' passed as %s, expecting %s",
								sCalledFunctions.back(), argSym->name(), MincTypeName(argValue.dataType()), MincTypeName(argSym->dataType()));
				}
				else compatible = true;
				break;
			default:
                minc_internal_error("%s() arg '%s' is an unhandled type!", sCalledFunctions.back(), argSym->name());
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
    TPRINT("NodeRet(%p): Evaluate value %p (child 0)\n", this, child(0));
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
	while ((bool)child(1)->exct()->value() == true) { /* condition */
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
	TPRINT("NodeDecl(%p) -- declaring variable '%s'\n", this, _symbolName);
	Symbol *sym = lookupSymbol(_symbolName, inCalledFunctionArgList ? ThisLevel : AnyLevel);
	if (!sym) {
		sym = installSymbol(_symbolName, NO);
		sym->value() = MincValue(this->_type);
	}
	else {
		if (sym->scope == current_scope()) {
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
			sym->value() = MincValue(this->_type);
		}
	}
	this->setSymbol(sym);
	return this;
}

Node *  NodeStructDef::doExct()
{
    TPRINT("NodeStructDef(%p) -- storing declaration for struct type '%s'\n", this, _typeName);
    if (current_scope() == 0) {    // until I allow nested structs
        sNewStructType = installType(_typeName, YES);  // all structs global for now
        if (sNewStructType) {
            TPRINT("-- walking element list\n");
            child(0)->exct();
            sNewStructType = NULL;
        }
    }
    else {
        minc_die("struct definitions only allowed in global scope for now");
    }
    return this;
}

Node *  NodeMemberDecl::doExct()
{
    TPRINT("NodeMemberDecl(%p) -- storing decl info for member '%s', type %s\n", this, _symbolName, MincTypeName(this->_type));
    assert(sNewStructType != NULL);
    sNewStructType->addElement(_symbolName, this->_type);
    return this;
}

Node *    NodeStructDecl::doExct()
{
    TPRINT("NodeStructDecl(%p) -- looking up type '%s'\n", this, _typeName);
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
                    minc_die("%s(): argument variable '%s' already used", sCalledFunctions.back(), _symbolName);
                }
                minc_warn("variable '%s' redefined - using existing one", _symbolName);
            }
            else {
                if (!inFunctionCall() && !inCalledFunctionArgList) {
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
	TPRINT("NodeFuncDecl(%p) -- declaring function '%s'\n", this, _symbolName);
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
	TPRINT("NodeFuncDef(%p): executing lookup node %p\n", this, child(0));
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

static void print_symbol(Symbol * s)
{
	TPRINT("Symbol %p: '%s' scope: %d type: %d\n", s, s->name(), s->scope, s->dataType());
}

