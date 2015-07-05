/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* A revised MinC, supporting lists, types and other fun things.
   Based heavily on the classic cmix version by Lars Graf.
   Doug Scott added the '#' and '//' comment parsing.

   John Gibson <johgibso at indiana dot edu>, 1/20/04
*/

/* This file holds the intermediate tree representation. */

#undef DEBUG

#include "minc_internal.h"
#include "handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* We maintain a stack of MAXSTACK lists, which we access when forming 
   user lists (i.e., {1, 2, "foo"}) and function argument lists.  Each
   element of this stack is a list, allocated and freed by push_list and
   pop_list.  <list> is an array of MincListElem structures, each having
   a type and a value, which is encoded in a MincValue union.  Nested lists
   and lists of mixed types are possible.
*/
static MincListElem *sMincList;
static int sMincListLen;
static MincListElem *list_stack[MAXSTACK];
static int list_len_stack[MAXSTACK];
static int list_stack_ptr;

static int sArgListLen;		// number of arguments passed to a user-declared function
static int sArgListIndex;	// used to walk passed-in args for user-declared functions

static bool inCalledFunctionArgList = false;
static const char *sCalledFunction;
static int sFunctionCallDepth = 0;	// level of actively-executing function calls

#undef DEBUG_TRACE
#if defined(DEBUG_TRACE) && defined(__cplusplus)
class Trace {
public:
	Trace(const char *func) : mFunc(func) {
		for (int n =0; n<sTraceDepth*2; ++n) { spaces[n] = ' '; }
		spaces[sTraceDepth] = '\0';
		rtcmix_print("%s%s -->\n", spaces, mFunc); ++sTraceDepth;
	}
	~Trace() { rtcmix_print("%s<-- %s\n", spaces, mFunc); --sTraceDepth; }
private:
	const char *mFunc;
	static int sTraceDepth;
	static char spaces[];
};
	
int Trace::sTraceDepth = 0;
char Trace::spaces[64];

#define ENTER() Trace __trace__(__FUNCTION__)
#else
#define ENTER()
#endif

#undef DEBUG_MEMORY
#ifdef DEBUG_MEMORY
static int numTrees = 0;
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
   "NodeOpAssign",
   "NodeName",
   "NodeAutoName",
   "NodeConstf",
   "NodeString",
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
   "NodeFuncDecl",
   "NodeBlock",
   "NodeNoop"
};

static const char *printNodeKind(NodeKind k)
{
	return s_NodeKinds[k];
}

/* prototypes for local functions */
static int cmp(MincFloat f1, MincFloat f2);
static Tree node(OpKind op, NodeKind kind);
static void push_list(void);
static void pop_list(void);
static const char *MincTypeName(MincDataType type);

static void copy_tree_tree(Tree tpdest, Tree tpsrc);
static void copy_sym_tree(Tree tpdest, Symbol *src);
static void copy_tree_sym(Symbol *dest, Tree tpsrc);
static void copy_tree_listelem(MincListElem *edest, Tree tpsrc);
static void copy_listelem_tree(Tree tpdest, MincListElem *esrc);
static void copy_listelem_elem(MincListElem *edest, MincListElem *esrc);

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

static MincList *
newList(int len)
{
	ENTER();
   MincList *list = (MincList *) emalloc(sizeof(MincList));
   if (list) {
	  list->len = len;
	  list->refcount = 0;
	  if (len > 0) {
         list->data = (MincListElem *) emalloc(len * sizeof(MincListElem));
		 if (!list->data) {
			efree(list);
			list = NULL;
		 }
		 else {
            memset(list->data, 0, len * sizeof(MincListElem));
		 }
	  }
	  else
         list->data = NULL;
      DPRINT2("newList: %p alloc'd at len %d\n", list, sMincListLen);
   }
   return list;
}

static void
resizeList(MincList *list, int newLen)
{
   int i;
   list->data = (MincListElem *) realloc(list->data, sizeof(MincListElem) * newLen);
   for (i = list->len; i < newLen; i++) {
	  list->data[i].type = MincFloatType;
	  list->data[i].val.number = 0.0;
   }
   list->len = newLen;
}

/* ========================================================================== */
/* Tree nodes */

static Tree
node(OpKind op, NodeKind kind)
{
   Tree tp;

   tp = (Tree) emalloc(sizeof(struct tree));
   if (tp == NULL)
      return NULL;
   tp->op = op;
   tp->kind = kind;
   tp->type = MincVoidType;
   tp->u.child[0] = NULL;    /* these clear entire <u> union */
   tp->u.child[1] = NULL;
   tp->u.child[2] = NULL;
   tp->u.child[3] = NULL;
   tp->v.list = NULL;
   tp->name = NULL;
#ifdef DEBUG_MEMORY
	++numTrees;
   DPRINT1("[%d trees in existence]\n", numTrees);
#endif
   return tp;
}


Tree
tnoop()
{
   Tree tp = node(OpFree, NodeNoop);

   DPRINT1("tnoop => NodeNoop %p\n", tp);
   return tp;
}


Tree
tseq(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeSeq);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tseq (%p, %p) => NodeSeq %p\n", e1, e2, tp);
   return tp;
}


Tree
top(OpKind op, Tree e1, Tree e2)
{
   Tree tp = node(op, NodeOperator);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("top (%d, %p, %p) => NodeOperator %p\n", op, e1, e2, tp);
   return tp;
}


Tree
tunop(OpKind op, Tree e1)
{
   Tree tp = node(op, NodeUnaryOperator);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;

   DPRINT3("tunop (%d, %p) => NodeUnaryOperator %p\n", op, e1, tp);
   return tp;
}


/* store a value into a variable */
Tree
tstore(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeStore);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tstore (%p, %p) => NodeStore %p\n", e1, e2, tp);
   return tp;
}


/* like tstore, but modify value before storing into variable */
Tree
topassign(Tree e1, Tree e2, OpKind op)
{
   Tree tp = node(op, NodeOpAssign);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("topassign, op=%d (%p, %p) => NodeOpAssign %p\n", op, e1, e2, tp);
   return tp;
}


/* looks up symbol name and get the symbol.
   converts symbol table entry into tree
   or initialize tree node to a symbol entry
*/
Tree
tname(const char *symbolName)
{
   Tree tp = node(OpFree, NodeName);
   if (tp == NULL)
      return NULL;

	tp->name = symbolName;

   DPRINT2("tname ('%s') => NodeName %p\n", symbolName, tp);
   return tp;
}

/* looks up symbol name and get the symbol, and auto-declares it if not found
 	converts symbol table entry into tree
 	or initialize tree node to a symbol entry
 */

Tree
tautoname(const char *symbolName)
{
	Tree tp = node(OpFree, NodeAutoName);
	if (tp == NULL)
		return NULL;
	
	tp->name = symbolName;
	
	DPRINT2("tautodecl ('%s') => NodeAutoName %p\n", symbolName, tp);
	return tp;
}

Tree
tstring(const char *str)
{
   Tree tp = node(OpFree, NodeString);
   if (tp == NULL)
      return NULL;

   tp->u.string = str;

   DPRINT2("tstring ('%s') => NodeString %p\n", str, tp);
   return tp;
}


Tree
tconstf(MincFloat num)
{
   Tree tp = node(OpFree, NodeConstf);
   if (tp == NULL)
      return NULL;

   tp->u.number = num;

   DPRINT2("tconstf (%f) => NodeConstf %p\n", num, tp);
   return tp;
}

Tree
targlistelem(Tree e1, Tree e2)
{
	Tree tp = node(OpFree, NodeArgListElem);
	if (tp == NULL)
		return NULL;
	
	tp->u.child[0] = e1;	// previous NodeArgListElem
	tp->u.child[1] = e2;	// payload (NodeDecl for arg)
	
	DPRINT3("targlistelem (%p, %p) => NodeArgListElem %p\n", e1, e2, tp);
	return tp;
}

Tree
targlist(Tree e1)
{
	Tree tp = node(OpFree, NodeArgList);
	if (tp == NULL)
		return NULL;
	
	tp->u.child[0] = e1;	// tail of NodeArgListElem linked list
	
	DPRINT2("targlist (%p) => NodeArgList %p\n", e1, tp);
	return tp;
}

Tree
treturn(Tree e1)
{
	Tree tp = node(OpFree, NodeRet);
	if (tp == NULL)
		return NULL;
	
	tp->u.child[0] = e1;	// Node containing RHS for return
	
	DPRINT2("treturn (%p) => NodeRet %p\n", e1, tp);
	return tp;
}

Tree tfuncseq(Tree e1, Tree e2)
{
	Tree tp = node(OpFree, NodeFuncSeq);
	if (tp == NULL)
		return NULL;
	
	tp->u.child[0] = e1;
	tp->u.child[1] = e2;
	
	DPRINT3("tfuncseq (%p, %p) => NodeFuncSeq %p\n", e1, e2, tp);
	return tp;
}

Tree
tfdef(Tree e1, Tree e2, Tree e3)
{
	Tree tp = node(OpFree, NodeFuncDef);
	if (tp == NULL)
		return NULL;
	
	tp->u.child[0] = e1;	// Lookup node
	tp->u.child[1] = e2;	// NodeArgList (argument symbol decls)
	tp->u.child[2] = e3;	// NodeFuncSeq function body (statements), which returns value
							// Right now, this last is handed to the Symbol and
							// then NULL'd here in exct()
	
	DPRINT4("tfdef (%p, %p, %p) => NodeFuncDef %p\n", e1, e2, e3, tp);
	return tp;
}

Tree
tcall(Tree args, const char *funcname)
{
   Tree tp = node(OpFree, NodeCall);
   if (tp == NULL)
      return NULL;

   tp->name = funcname;
   tp->u.child[0] = args;

   DPRINT3("tcall (%p, '%s') => NodeCall %p\n", args, funcname, tp);
   return tp;
}


Tree
tcand(Tree test1, Tree test2)
{
   Tree tp = node(OpFree, NodeAnd);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = test1;
   tp->u.child[1] = test2;

   DPRINT3("tcand (%p, %p) => NodeAnd %p\n", test1, test2, tp);
   return tp;
}


Tree
tcor(Tree test1, Tree test2)
{
   Tree tp = node(OpFree, NodeOr);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = test1;
   tp->u.child[1] = test2;

   DPRINT3("tcor (%p, %p) => NodeOr %p\n", test1, test2, tp);
   return tp;
}


Tree
tnot(Tree test1)
{
   Tree tp = node(OpFree, NodeNot);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = test1;

   DPRINT2("tnot (%p) => NodeNot %p\n", test1, tp);
   return tp;
}


Tree
trel(OpKind op, Tree e1, Tree e2)
{
   Tree tp = node(op, NodeRelation);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("trel (%d, %p, %p) => NodeRelation %p\n", op, e1, e2, tp);
   return tp;
}


/* Create list: either an argument list or a user array.  Why do we
   not separate these two things?  Because at the time when we need
   to push the list elements onto a stack, we don't know whether they
   form part of a user list or an argument list.
*/
Tree
tlist(Tree e1)
{
   Tree tp = node(OpFree, NodeList);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;		// tail of NodeListElem linked list

   DPRINT2("tlist (%p) => NodeList %p\n", e1, tp);
   return tp;
}


Tree
tlistelem(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeListElem);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;		// previous elem
   tp->u.child[1] = e2;		// payload (contents of exp)

   DPRINT3("tlistelem (%p, %p) => NodeListElem %p\n", e1, e2, tp);
   return tp;
}


Tree
temptylistelem()
{
   Tree tp = node(OpFree, NodeEmptyListElem);

   DPRINT1("temptylistelem => NodeEmptyListElem %p\n", tp);
   return tp;
}


Tree
tsubscriptread(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeSubscriptRead);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tsubscriptread (%p, %p) => NodeSubscriptRead %p\n", e1, e2, tp);
   return tp;
}


Tree
tsubscriptwrite(Tree e1, Tree e2, Tree e3)
{
   Tree tp = node(OpFree, NodeSubscriptWrite);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;

   DPRINT4("tsubscriptwrite (%p, %p, %p) => NodeSubscriptWrite %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
tif(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeIf);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tif (%p, %p) => NodeIf %p\n", e1, e2, tp);
   return tp;
}


Tree
tifelse(Tree e1, Tree e2, Tree e3)
{
   Tree tp = node(OpFree, NodeIfElse);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;

   DPRINT4("tifelse (%p, %p, %p) => NodeIfElse %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
tfor(Tree e1, Tree e2, Tree e3, Tree e4)
{
   Tree tp = node(OpFree, NodeFor);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;
   tp->u.child[3] = e4;

   DPRINT4("tfor (%p, %p, %p, <e4>) => NodeFor %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
twhile(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeWhile);
   if (tp == NULL)
      return NULL;

   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("twhile (%p, %p) => NodeWhile %p\n", e1, e2, tp);
   return tp;
}

Tree tdecl(const char *name, MincDataType type)
{
	Tree tp = node(OpFree, NodeDecl);
	if (tp == NULL)
		return NULL;
	tp->name = name;
	tp->type = type;

	DPRINT2("tdecl ('%s') => NodeDecl %p\n", name, tp);
	return tp;
}

Tree tfdecl(const char *name, MincDataType type)
{
	Tree tp = node(OpFree, NodeFuncDecl);
	if (tp == NULL)
		return NULL;
	tp->name = name;
	tp->type = type;
	
	DPRINT2("tfdecl ('%s') => NodeFuncDecl %p\n", name, tp);
	return tp;
}

Tree tblock(Tree e1)
{
	Tree tp = node(OpFree, NodeBlock);
	if (tp == NULL)
		return NULL;

	tp->u.child[0] = e1;
	DPRINT2("tblock (%p) => NodeBlock %p\n", e1, tp);
	return tp;
}

/* ========================================================================== */
/* Operators */

/* ---------------------------------------------------------- do_op_string -- */
static void
do_op_string(Tree tp, const char *str1, const char *str2, OpKind op)
{
	ENTER();
   char *s;
   unsigned long   len;

   switch (op) {
      case OpPlus:   /* concatenate */
         len = (strlen(str1) + strlen(str2)) + 1;
         s = (char *) emalloc(sizeof(char) * len);
         if (s == NULL)
            return;
         strcpy(s, str1);
         strcat(s, str2);
         tp->v.string = s;
         // printf("str1=%s, str2=%s len=%d, s=%s\n", str1, str2, len, s);
         break;
      case OpMinus:
      case OpMul:
      case OpDiv:
      case OpMod:
      case OpPow:
      case OpNeg:
         minc_warn("unsupported operation on a string");
         return;
      default:
         minc_internal_error("invalid string operator");
         break;
   }
   tp->type = MincStringType;
}


/* ------------------------------------------------------------- do_op_num -- */
static void
do_op_num(Tree tp, const MincFloat val1, const MincFloat val2, OpKind op)
{
	ENTER();
   switch (op) {
      case OpPlus:
         tp->v.number = val1 + val2;
         break;
      case OpMinus:
         tp->v.number = val1 - val2;
         break;
      case OpMul:
         tp->v.number = val1 * val2;
         break;
      case OpDiv:
         tp->v.number = val1 / val2;
         break;
      case OpMod:
         tp->v.number = (MincFloat) ((long) val1 % (long) val2);
         break;
      case OpPow:
         tp->v.number = pow(val1, val2);
         break;
      case OpNeg:
         tp->v.number = -val1;        /* <val2> ignored */
         break;
      default:
         minc_internal_error("invalid numeric operator");
         break;
   }
   tp->type = MincFloatType;
}


/* ------------------------------------------------------ do_op_handle_num -- */
static void
do_op_handle_num(Tree tp, const MincHandle val1, const MincFloat val2,
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
         tp->v.handle = minc_binop_handle_float(val1, val2, op);
         ref_handle(tp->v.handle);
         break;
      case OpNeg:
         tp->v.handle = minc_binop_handle_float(val1, -1.0, OpMul);	// <val2> ignored
         ref_handle(tp->v.handle);
         break;
      default:
         minc_internal_error("invalid operator for handle and number");
         break;
   }
   tp->type = MincHandleType;
}


/* ------------------------------------------------------ do_op_num_handle -- */
static void
do_op_num_handle(Tree tp, const MincFloat val1, const MincHandle val2,
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
         tp->v.handle = minc_binop_float_handle(val1, val2, op);
         ref_handle(tp->v.handle);
         break;
      case OpNeg:
         /* fall through */
      default:
         minc_internal_error("invalid operator for handle and number");
         break;
   }
   tp->type = MincHandleType;
}


/* --------------------------------------------------- do_op_handle_handle -- */
static void
do_op_handle_handle(Tree tp, const MincHandle val1, const MincHandle val2,
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
		tp->v.handle = minc_binop_handles(val1, val2, op);
        ref_handle(tp->v.handle);
		break;
	case OpNeg:
	default:
		minc_internal_error("invalid binary handle operator");
		break;
	}
	if (tp->v.handle)
		tp->type = MincHandleType;
}


/* ---------------------------------------------------- do_op_list_iterate -- */
/* Iterate over <child>'s list, performing the operation specified by <op>,
   using the scalar <val>, for each list element.  Store the result into a
   new list for <tp>, so that child's list is unchanged.
*/
static void
do_op_list_iterate(Tree tp, Tree child, const MincFloat val, const OpKind op)
{
	ENTER();
   int i;
   MincListElem *dest;
   const MincList *srcList = child->v.list;
   const int len = srcList->len;
   MincListElem *src = srcList->data;
   MincList *destList = newList(len);
   if (destList == NULL)
      return;
   dest = destList->data;
   assert(len >= 0);
   switch (op) {
      case OpPlus:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = src[i].val.number + val;
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpMinus:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = src[i].val.number - val;
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpMul:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = src[i].val.number * val;
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpDiv:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = src[i].val.number / val;
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpMod:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = (MincFloat) ((long) src[i].val.number
                                                            % (long) val);
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpPow:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = (MincFloat) pow((double) src[i].val.number,
                                                                  (double) val);
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      case OpNeg:
         for (i = 0; i < len; i++) {
            if (src[i].type == MincFloatType)
               dest[i].val.number = -src[i].val.number;    /* <val> ignored */
            else
               dest[i].val = src[i].val;
            dest[i].type = src[i].type;
         }
         break;
      default:
         for (i = 0; i < len; i++)
            dest[i].val.number = 0.0;
         minc_internal_error("invalid list operator");
         break;
   }
   assert(tp->type == MincVoidType);	// are we ever overwriting these?
   tp->type = MincListType;
   assert(tp->v.list == NULL);
   tp->v.list = destList;
   DPRINT3("do_op_list_iterate: list %p refcount %d -> %d\n", destList, destList->refcount, destList->refcount+1);
   ++destList->refcount;
}


/* --------------------------------------------------------- exct_operator -- */
static void
exct_operator(Tree tp, OpKind op)
{
	ENTER();
   Tree child0, child1;

   child0 = exct(tp->u.child[0]);
   child1 = exct(tp->u.child[1]);
   switch (child0->type) {
      case MincFloatType:
         switch (child1->type) {
            case MincFloatType:
               do_op_num(tp, child0->v.number, child1->v.number, op);
               break;
            case MincStringType:
               {
                  char buf[64];
                  snprintf(buf, 64, "%g", child0->v.number);
                  do_op_string(tp, buf, child1->v.string, op);
               }
               break;
            case MincHandleType:
               do_op_num_handle(tp, child0->v.number, child1->v.handle, op);
               break;
            case MincListType:
               /* Check for nonsensical ops. */
               if (op == OpMinus)
                  minc_warn("can't subtract a list from a number");
               else if (op == OpDiv)
                  minc_warn("can't divide a number by a list");
               else
                  do_op_list_iterate(tp, child1, child0->v.number, op);
               break;
            default:
               minc_internal_error("exct_operator: invalid type");
               break;
         }
         break;
      case MincStringType:
         switch (child1->type) {
            case MincFloatType:
               {
                  char buf[64];
                  snprintf(buf, 64, "%g", child1->v.number);
                  do_op_string(tp, child0->v.string, buf, op);
               }
               break;
            case MincStringType:
               do_op_string(tp, child0->v.string, child1->v.string, op);
               break;
            case MincHandleType:
               minc_warn("can't operate on a string and a handle");
               break;
            case MincListType:
               minc_warn("can't operate on a string and a list");
               break;
            default:
               minc_internal_error("exct_operator: invalid type");
               break;
         }
         break;
      case MincHandleType:
         switch (child1->type) {
            case MincFloatType:
               do_op_handle_num(tp, child0->v.handle, child1->v.number, op);
               break;
            case MincStringType:
               minc_warn("can't operate on a string and a handle");
               break;
            case MincHandleType:
               do_op_handle_handle(tp, child0->v.handle, child1->v.handle, op);
               break;
            case MincListType:
               minc_warn("can't operate on a list and a handle");
               break;
            default:
               minc_internal_error("exct_operator: invalid type");
               break;
         }
         break;
      case MincListType:
         switch (child1->type) {
            case MincFloatType:
               do_op_list_iterate(tp, child0, child1->v.number, op);
               break;
            case MincStringType:
               minc_warn("can't operate on a string");
               break;
            case MincHandleType:
               minc_warn("can't operate on a handle");
               break;
            case MincListType:
               minc_warn("can't operate on two lists");
               break;
            default:
               minc_internal_error("exct_operator: invalid type");
               break;
         }
         break;
      default:
         minc_internal_error("exct_operator: invalid type");
         break;
   }
}


/* --------------------------------------------------------- exct_relation -- */
static void
exct_relation(Tree tp)
{
	ENTER();
   Tree tp0 = exct(tp->u.child[0]);
   Tree tp1 = exct(tp->u.child[1]);

   tp->type = MincFloatType;

   if (tp0->type != tp1->type) {
      minc_warn("attempt to compare objects having different types");
      tp->v.number = 0.0;
      return;
   }

   switch (tp->op) {
      case OpEqual:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) == 0)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
               break;
            case MincStringType:
               if (strcmp(tp0->v.string, tp1->v.string) == 0)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
               break;
            default:
               goto unsupported_type;
               break;
         }
         break;
      case OpNotEqual:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) == 0)
                  tp->v.number = 0.0;
               else
                  tp->v.number = 1.0;
               break;
            case MincStringType:
               if (strcmp(tp0->v.string, tp1->v.string) == 0)
                  tp->v.number = 0.0;
               else
                  tp->v.number = 1.0;
               break;
            default:
               goto unsupported_type;
               break;
         }
         break;
      case OpLess:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) == -1)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
               break;
            default:
               goto unsupported_type;
               break;
         }
         break;
      case OpGreater:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) == 1)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
               break;
            default:
               goto unsupported_type;
               break;
         }
         break;
      case OpLessEqual:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) <= 0)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
               break;
            default:
               goto unsupported_type;
               break;
         }
         break;
      case OpGreaterEqual:
         switch (tp0->type) {
            case MincFloatType:
               if (cmp(tp0->v.number, tp1->v.number) >= 0)
                  tp->v.number = 1.0;
               else
                  tp->v.number = 0.0;
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
   return;
unsupported_type:
   minc_internal_error("can't compare this type of object");
}


/* --------------------------------------------------------- exct_opassign -- */
static void
exct_opassign(Tree tp, OpKind op)
{
	ENTER();
   Tree tp0 = exct(tp->u.child[0]);
   Tree tp1 = exct(tp->u.child[1]);

   if (tp0->u.symbol->type != MincFloatType || tp1->type != MincFloatType) {
      minc_warn("can only use '%c=' with numbers",
                           op == OpPlus ? '+' : (op == OpMinus ? '-'
                                              : (op == OpMul ? '*' : '/')));
//FIXME: Is this correct?
//      memcpy(&tp->v, &tp0->u.symbol->v, sizeof(MincValue));
//      tp->type = tp0->type;
      copy_sym_tree(tp, tp0->u.symbol);
      return;
   }

   switch (tp->op) {
      case OpPlus:
         tp0->u.symbol->v.number += tp1->v.number;
         break;
      case OpMinus:
         tp0->u.symbol->v.number -= tp1->v.number;
         break;
      case OpMul:
         tp0->u.symbol->v.number *= tp1->v.number;
         break;
      case OpDiv:
         tp0->u.symbol->v.number /= tp1->v.number;
         break;
      default:
         minc_internal_error("exct: tried to execute invalid NodeOpAssign");
         break;
   }
   tp0->u.symbol->type = tp1->type;
   tp->v.number = tp0->u.symbol->v.number;
   tp->type = tp1->type;
}


/* --------------------------------------------------- exct_subscript_read -- */
static void
exct_subscript_read(Tree tp)
{
	ENTER();
	exct(tp->u.child[0]);         /* lookup target */
   exct(tp->u.child[1]);
   if (tp->u.child[1]->type == MincFloatType) {
      if (tp->u.child[0]->u.symbol->type == MincListType) {
         MincListElem elem;
         MincFloat fltindex = tp->u.child[1]->v.number;
         int index = (int) fltindex;
         MincFloat frac = fltindex - index;
		 MincList *theList = tp->u.child[0]->u.symbol->v.list;
		 int len = 0;
		  if (theList == NULL) {
            minc_die("attempt to index an empty list");
			  return;
		  }
         len = theList->len;
         if (fltindex < 0.0) {    /* -1 means last element */
            if (fltindex <= -2.0)
               minc_warn("negative index ... returning last element");
            index = len - 1;
            fltindex = (MincFloat) index;
         }
         else if (fltindex > (MincFloat) (len - 1)) {
            minc_warn("attempt to index past the end of list ... "
                                                "returning last element");
            index = len - 1;
            fltindex = (MincFloat) index;
         }
		 elem.type = MincVoidType;
		 copy_listelem_elem(&elem, &theList->data[index]);		 	

         /* do linear interpolation for float items */
         if (elem.type == MincFloatType && frac > 0.0 && index < len - 1) {
            MincListElem elem2 = theList->data[index + 1];
            if (elem2.type == MincFloatType)
               tp->v.number = elem.val.number
                        + (frac * (elem2.val.number - elem.val.number));
            else  /* can't interpolate btw. a number and another type */
               tp->v.number = elem.val.number;
            tp->type = elem.type;
         }
         else {
            copy_listelem_tree(tp, &elem);
         }
		 clear_elem(&elem);
      }
      else
         minc_die("attempt to index a variable that's not a list");
   }
   else
      minc_die("list index must be a number");
}


/* -------------------------------------------------- exct_subscript_write -- */
static void
exct_subscript_write(Tree tp)
{
	ENTER();
   exct(tp->u.child[0]);         /* lookup target */
   exct(tp->u.child[1]);         /* index */
   exct(tp->u.child[2]);         /* expression to store */
   if (tp->u.child[1]->type == MincFloatType) {
      if (tp->u.child[0]->u.symbol->type == MincListType) {
         int len = 0;
		 MincList *theList = tp->u.child[0]->u.symbol->v.list;
         MincFloat fltindex = tp->u.child[1]->v.number;
         int index = (int) fltindex;
         if (fltindex - (MincFloat) index > 0.0)
            minc_warn("list index must be integer ... correcting");
		 if (theList != NULL) {
        	len = theList->len;
        	assert(len >= 0);    /* NB: okay to have zero-length list */
		 }
         if (index == -1)     /* means last element */
            index = len > 0 ? len - 1 : 0;
         else if (index >= len) {
            /* resize list */
            int i, newslots, oldlen = len;
            newslots = len > 0 ? (index - (len - 1)) : index + 1;
            len += newslots;
			if (len < 0) {
			    minc_die("list array subscript exceeds integer size limit!");
			}
			if (theList == NULL)
				tp->u.child[0]->u.symbol->v.list = theList = newList(len);
			else
				resizeList(theList, len);
			DPRINT2("exct_subscript_write: MincList %p expanded to len %d\n",
					theList->data, len);
			// Ref the list if just allocated.
			if (theList->refcount == 0)
				theList->refcount = 1;
  			 DPRINT1("list %p refcount = 1\n", theList);
         }
         copy_tree_listelem(&theList->data[index], tp->u.child[2]);
		 assert(theList->data[index].type == tp->u.child[2]->type);
         copy_tree_tree(tp, tp->u.child[2]);
      }
      else
         minc_die("attempt to index a variable that's not a list");
   }
   else
      minc_die("list index must be a number");
}


/* ========================================================================== */
/* Tree execution and disposal */

/* ------------------------------------------------------ check_list_count -- */
/* This protects us against a situation that can arise due to our use of
   '{' and '}' to delimit both statements and list contents.  If you write 
   the following in a script, it will quickly chew through all available
   memory, as it allocates a zero-length block for an empty list on each
   iteration.

      while (1) {}

   This function prevents this from going on for too many thousands of
   iterations.
*/
#define MAX_LISTS 50000

static int
check_list_count()
{
   static int list_count = 0;
   if (++list_count > MAX_LISTS) {
      minc_die("Bailing out due to suspected infinite loop on "
               "empty code block\n(e.g., \"while (1) {}\").");
      return -1;
   }
   return 0;
}


/* ------------------------------------------------------------------ exct -- */
/* This recursive function interprets the intermediate code.
*/
Tree
exct(Tree tp)
{
   if (tp == NULL || was_rtcmix_error())
      return NULL;

	DPRINT2("exct (enter %s, tp=%p)\n", printNodeKind(tp->kind), tp);
   switch (tp->kind) {
      case NodeConstf:
         tp->type = MincFloatType;
         tp->v.number = tp->u.number;
         break;
      case NodeString:
         tp->type = MincStringType;
         tp->v.string = tp->u.string;
         break;
      case NodeName:
      case NodeAutoName:
         /* look up the symbol */
		   if (tp->kind == NodeName) {
			   tp->u.symbol = lookup(tp->name, AnyLevel);
		   }
		   else if (tp->kind == NodeAutoName) {
			   tp->u.symbol = lookupOrAutodeclare(tp->name);
		   }
		   else {
			   minc_internal_error("NodeName/NodeAutoName exct: illegal node kind: %d", tp->kind);
		   }
         if (tp->u.symbol) {
			DPRINT1("exct NodeName/NodeAutoName: symbol %p\n", tp->u.symbol);
        	 /* also assign the symbol's value into tree's value field */
         	DPRINT1("exct NodeName/NodeAutoName: copying value from symbol '%s' to us\n", tp->u.symbol->name);
         	copy_sym_tree(tp, tp->u.symbol);
         	tp->name = tp->u.symbol->name;		// for debugging -- not used
		 	assert(tp->type == tp->u.symbol->type);
		 }
		 else {
			 // FIXME: install id w/ value of 0, then warn??
			 minc_die("'%s' is not declared", tp->name);
		 }
         break;
      case NodeListElem:
         DPRINT1("NodeListElem exct'ing Tree link %p\n", tp->u.child[0]);
         exct(tp->u.child[0]);
		 if (sMincListLen == MAXDISPARGS) {
            minc_die("exceeded maximum number of items for a list");
		 }
         else {
			DPRINT2("NodeListElem %p evaluating payload child Tree %p\n", tp, tp->u.child[1]);
            Tree tmp = exct(tp->u.child[1]);
            /* Copy entire MincValue union from expr to tp and to stack. */
			DPRINT1("NodeListElem %p copying child value into self and stack\n", tp);
            copy_tree_tree(tp, tmp);
            copy_tree_listelem(&sMincList[sMincListLen], tmp);
			assert(sMincList[sMincListLen].type == tmp->type);
            sMincListLen++;
			 DPRINT2("NodeListElem: list at level %d now len %d\n", list_stack_ptr, sMincListLen);
         }
         break;
      case NodeEmptyListElem:
         /* do nothing */
         break;
      case NodeList:
         push_list();
         exct(tp->u.child[0]);     /* NB: increments sMincListLen */
         {
		    MincList *theList;
			int i;
            if (check_list_count() < 0)
               return tp;
			theList = newList(sMincListLen);
            if (theList == NULL)
               return NULL;
			if (tp->type == MincListType && tp->v.list != NULL)
			   unref_value_list(&tp->v);
            tp->type = MincListType;
            tp->v.list = theList;
  			DPRINT1("MincList %p assigned to self\n", theList);
			theList->refcount = 1;
  			DPRINT("MincList refcount = 1\n");
			// Copy from stack list into tree list.
			for (i = 0; i < sMincListLen; ++i)
            	copy_listelem_elem(&theList->data[i], &sMincList[i]);
         }
         pop_list();
         break;
      case NodeSubscriptRead:
         exct_subscript_read(tp);
         break;
      case NodeSubscriptWrite:
         exct_subscript_write(tp);
         break;
      case NodeCall:
		push_list();
		{
			Symbol *funcSymbol = lookup(tp->name, GlobalLevel);
			if (funcSymbol) {
				sCalledFunction = tp->name;
				/* The function's definition node was stored on the symbol at declaration time.
					However, if a function was called on a non-function symbol, the tree will be NULL.
				 */
				Tree funcDef = funcSymbol->tree;
				if (funcDef) {
					DPRINT1("exct NodeCall: func def = %p\n", funcDef);
					DPRINT1("exct NodeCall: exp decl list = %p\n", tp->u.child[0]);
					exct(tp->u.child[0]);	// execute arg expression list
					++sFunctionCallDepth;
					push_function_stack();
					push_scope();
					/* The exp list is copied to the symbols for the function's arg list. */
					exct(funcDef->u.child[1]);
					Tree temp = NULL;
					int savedScope = current_scope();
					int savedCallDepth = sFunctionCallDepth;
					DPRINT1("exct NodeCall: executing function block node %p\n", funcDef->u.child[2]);
					try {
						temp = exct(funcDef->u.child[2]);
					}
					catch (Tree returned) {	// This catches return statements!
						DPRINT("NodeCall caught return stmt throw\n");
						temp = returned;
						sFunctionCallDepth = savedCallDepth;
						restore_scope(savedScope);
					}
					DPRINT("NodeCall copying def exct results into self\n");
					copy_tree_tree(tp, temp);
					pop_scope();
					pop_function_stack();
					--sFunctionCallDepth;
				}
				else {
					minc_die("'%s' is not a function", funcSymbol->name);
				}
				sCalledFunction = NULL;
			}
			else {
				exct(tp->u.child[0]);
				MincListElem retval;
				int result = call_builtin_function(tp->name, sMincList, sMincListLen,
																	 &retval);
				if (result < 0) {
					result = call_external_function(tp->name, sMincList, sMincListLen,
																	 &retval);
				}
				copy_listelem_tree(tp, &retval);
				assert(tp->type == retval.type);
				clear_elem(&retval);
				if (result != 0) {
					set_rtcmix_error(result);	// store fatal error from RTcmix layer (EMBEDDED ONLY)
				}
			}
		}
		pop_list();
		break;
      case NodeStore:
		 /* N.B. Now that symbol lookup is part of tree, this happens in
		    the NodeName stored as child[0] */
         exct(tp->u.child[0]);
		 /* evaluate RHS expression */
         exct(tp->u.child[1]);
         DPRINT2("exct NodeStore: copying value from rhs (%p) to lhs's symbol (%p)\n", tp->u.child[1], tp->u.child[0]->u.symbol);
		/* Copy entire MincValue union from expr to id sym and to tp. */
         copy_tree_sym(tp->u.child[0]->u.symbol, tp->u.child[1]);
		 assert(tp->u.child[0]->u.symbol->type == tp->u.child[1]->type);
         DPRINT2("exct NodeStore: copying value from rhs (%p) to here (%p)\n", tp->u.child[1], tp);
         copy_tree_tree(tp, tp->u.child[1]);
         assert(tp->type == tp->u.child[1]->type);
         break;
      case NodeOpAssign:
         exct_opassign(tp, tp->op);
         break;
      case NodeNot:
         tp->type = MincFloatType;
         if (cmp(0.0, exct(tp->u.child[0])->v.number) == 0)
            tp->v.number = 1.0;
         else
            tp->v.number = 0.0;
         break;
      case NodeAnd:
         tp->type = MincFloatType;
         tp->v.number = 0.0;
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0) {
            if (cmp(0.0, exct(tp->u.child[1])->v.number) != 0) {
               tp->type = MincFloatType;
               tp->v.number = 1.0;
            }
         }
         break;
      case NodeRelation:
         exct_relation(tp);
         break; /* switch NodeRelation */
      case NodeOperator:
         exct_operator(tp, tp->op);
         break; /* switch NodeOperator */
      case NodeUnaryOperator:
         tp->type = MincFloatType;
         if (tp->op == OpNeg)
            tp->v.number = -exct(tp->u.child[0])->v.number;
         break;
      case NodeOr:
         tp->type = MincFloatType;
         tp->v.number = 0.0;
         if ((cmp(0.0, exct(tp->u.child[0])->v.number) != 0) ||
             (cmp(0.0, exct(tp->u.child[1])->v.number) != 0)) {
            tp->v.number = 1.0;
         }
         break;
      case NodeIf:
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         break;
      case NodeIfElse:
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         else
            exct(tp->u.child[2]);
         break;
      case NodeWhile:
         while (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         break;
      case NodeArgList:
		   sArgListLen = 0;
		   sArgListIndex = 0;	// reset to walk list
		   inCalledFunctionArgList = true;
		   DPRINT1("exct NodeArgList: walking function '%s' arg decl/copy list\n", sCalledFunction);
		   exct(tp->u.child[0]);
		   inCalledFunctionArgList = false;
         break;
      case NodeArgListElem:
		   ++sArgListLen;
		   exct(tp->u.child[0]);	// work our way to the front of the list
		   exct(tp->u.child[1]);	// run the arg decl
		   {
		   // Symbol associated with this function argument
		   Symbol *argSym = tp->u.child[1]->u.symbol;
		   if (sMincListLen > sArgListLen) {
			   minc_die("%s() takes %d arguments but was passed %d!", sCalledFunction, sArgListLen, sMincListLen);
		   }
		   else if (sArgListIndex >= sMincListLen) {
			   minc_warn("%s(): arg '%s' not provided - defaulting to 0", sCalledFunction, argSym->name);
			   /* Copy zeroed MincValue union to us and then to sym. */
			   MincListElem zeroElem;
			   zeroElem.type = argSym->type;
			   memset(&zeroElem.val, 0, sizeof(MincValue));
			   copy_listelem_tree(tp, &zeroElem);
			   copy_tree_sym(argSym, tp);
			   ++sArgListIndex;
		   }
		   /* compare stored NodeName with user-passed arg */
		   else {
			   // Pre-cached argument value from caller
			   MincListElem *argValue = &sMincList[sArgListIndex];
			   bool compatible = false;
			   switch (argValue->type) {
				   case MincFloatType:
				   case MincStringType:
					   if (argSym->type != argValue->type) {
						   minc_die("%s arg '%s' passed as %s, expecting %s",
									sCalledFunction, argSym->name, MincTypeName(argValue->type), MincTypeName(argSym->type));
					   }
					   else compatible = true;
					   break;
				   case MincHandleType:
				   case MincListType:
					   if (argSym->type != MincHandleType && argSym->type != MincListType) {
						   minc_die("%s arg '%s' passed as %s, expecting %s",
									sCalledFunction, argSym->name, MincTypeName(argValue->type), MincTypeName(argSym->type));
					   }
					   else compatible = true;
					   break;
				   default:
					   assert(argValue->type != MincVoidType);
					   break;
			   }
			   if (compatible) {
				   /* Copy passed-in arg's MincValue union to us and then to sym. */
				   copy_listelem_tree(tp, argValue);
				   copy_tree_sym(argSym, tp);
			   }
			   ++sArgListIndex;
		   }
		   }
		   break;
      case NodeRet:
         exct(tp->u.child[0]);
         copy_tree_tree(tp, tp->u.child[0]);
         assert(tp->type == tp->u.child[0]->type);
         DPRINT("NodeRet throwing for return stmt\n");
         throw tp;	// Cool, huh?  Throws this node's body out to function's endpoint!
         break;
      case NodeFuncSeq:
		   exct(tp->u.child[0]);
		   exct(tp->u.child[1]);
		   copy_tree_tree(tp, tp->u.child[1]);
		   assert(tp->type == tp->u.child[1]->type);
         break;
      case NodeNoop:
         /* do nothing */
         break;
      case NodeFor:
         exct(tp->u.child[0]);         /* init */
         while (cmp(0.0, exct(tp->u.child[1])->v.number) != 0) { /* condition */
            exct(tp->u.child[3]);      /* execute block */
            exct(tp->u.child[2]);      /* prepare for next iteration */
         }
         break;
      case NodeSeq:
         exct(tp->u.child[0]);
         exct(tp->u.child[1]);
         break;
	   case NodeBlock:				// NodeBlock returns void
		   push_scope();
		   exct(tp->u.child[0]);
		   pop_scope();
		   break;
	   case NodeDecl:
	   	{
		const char *name = tp->name;		// as set by NodeDecl creator
		DPRINT1("-- declaring variable '%s'\n", name);
		Symbol *sym = lookup(name, inCalledFunctionArgList ? ThisLevel : AnyLevel);
		if (!sym) {
		   sym = install(name);
		   sym->type = tp->type;
		}
		else {
		   if (sym->scope == current_scope()) {
			   if (inCalledFunctionArgList) {
				   minc_die("argument variable '%s' already used", name);
			   }
			   minc_warn("variable '%s' redefined - using existing one", name);
		   }
		   else {
			   if (sFunctionCallDepth == 0) {
			   	   minc_warn("variable '%s' also defined at enclosing scope", name);
			   }
			   sym = install(name);
			   sym->type = tp->type;
		   }
		}
		tp->u.symbol = sym;
		}
		break;
	   case NodeFuncDecl:
	   {
		   const char *name = tp->name;		// as set by NodeDecl creator
		   DPRINT1("-- declaring function '%s'\n", name);
		   assert(current_scope() == 0);	// until I allow nested functions
		   Symbol *sym = lookup(name, GlobalLevel);	// only look at current global level
		   if (sym == NULL) {
			   sym = install(name);		// all functions global for now
			   sym->type = tp->type;
			   tp->u.symbol = sym;
		   }
			else {
			   minc_die("function %s() is already declared", name);
			}
	   }
		break;
	   case NodeFuncDef:
	   {
		   // Look up symbol for function, and bind this FuncDef node to it.
		   DPRINT1("exct NodeFuncDef: executing lookup node %p\n", tp->u.child[0]);
		   exct(tp->u.child[0]);
		   assert(tp->u.child[0]->u.symbol != NULL);
		   tp->u.child[0]->u.symbol->tree = tp;
	   }
		   break;
      default:
         minc_internal_error("exct: tried to execute invalid node '%s'", printNodeKind(tp->kind));
         break;
   } /* switch kind */

	DPRINT2("exct (exit %s, tp=%p)\n", printNodeKind(tp->kind), tp);
   return tp;
}

static void
clear_list(MincListElem *list, int len)
{
	ENTER();
	int i;
	for (i = 0; i < len; ++i) {
		clear_elem(&list[i]);
	}
}

static void
push_list()
{
	ENTER();
   if (list_stack_ptr >= MAXSTACK)
      minc_die("stack overflow: too many nested function calls");
   list_stack[list_stack_ptr] = sMincList;
   list_len_stack[list_stack_ptr++] = sMincListLen;
   sMincList = (MincListElem *) calloc(MAXDISPARGS, sizeof(MincListElem));
   DPRINT3("push_list: sMincList=%p at stack level %d, len %d\n", sMincList, list_stack_ptr, sMincListLen);
   sMincListLen = 0;
}


static void
pop_list()
{
	ENTER();
   DPRINT1("pop_list: sMincList=%p\n", sMincList);
   clear_list(sMincList, MAXDISPARGS);
   efree(sMincList);
   if (list_stack_ptr == 0)
      minc_die("stack underflow");
   sMincList = list_stack[--list_stack_ptr];
   sMincListLen = list_len_stack[list_stack_ptr];
	DPRINT3("pop_list: now at sMincList=%p, stack level %d, len %d\n", sMincList, list_stack_ptr, sMincListLen);
}

static void copy_value(MincValue *dest, MincDataType destType,
                       MincValue *src, MincDataType srcType)
{
	ENTER();
   if (srcType == MincHandleType && src->handle != NULL) {
      ref_handle(src->handle);	// ref before unref
   }
   else if (srcType == MincListType) {
#ifdef DEBUG_MEMORY
      DPRINT3("list %p refcount %d -> %d\n", src->list, src->list->refcount, src->list->refcount+1);
#endif
      ++src->list->refcount;
   }
   if (destType == MincHandleType && dest->handle != NULL) {
      DPRINT("\toverwriting existing Handle value\n");
      unref_handle(dest->handle);	// overwriting handle, so unref
   }
   else if (destType == MincListType) {
      DPRINT("\toverwriting existing MincList value\n");
      unref_value_list(dest);
   }
   memcpy(dest, src, sizeof(MincValue));
}

/* This copies a Tree node's value and handles ref counting when necessary */
static void
copy_tree_tree(Tree tpdest, Tree tpsrc)
{
   DPRINT2("copy_tree_tree(%p, %p)\n", tpdest, tpsrc);
   copy_value(&tpdest->v, tpdest->type, &tpsrc->v, tpsrc->type);
   tpdest->type = tpsrc->type;
}

/* This copies a Symbol's value and handles ref counting when necessary */
static void
copy_sym_tree(Tree tpdest, Symbol *src)
{
   DPRINT2("copy_sym_tree(%p, %p)\n", tpdest, src);
   copy_value(&tpdest->v, tpdest->type, &src->v, src->type);
   tpdest->type = src->type;
}

static void
copy_tree_sym(Symbol *dest, Tree tpsrc)
{
   DPRINT2("copy_tree_sym(%p, %p)\n", dest, tpsrc);
   copy_value(&dest->v, dest->type, &tpsrc->v, tpsrc->type);
   dest->type = tpsrc->type;
}

static void
copy_tree_listelem(MincListElem *dest, Tree tpsrc)
{
   DPRINT2("copy_tree_listelem(%p, %p)\n", dest, tpsrc);
   copy_value(&dest->val, dest->type, &tpsrc->v, tpsrc->type);
   dest->type = tpsrc->type;
}

static void
copy_listelem_tree(Tree tpdest, MincListElem *esrc)
{
   DPRINT2("copy_listelem_tree(%p, %p)\n", tpdest, esrc);
   copy_value(&tpdest->v, tpdest->type, &esrc->val, esrc->type);
   tpdest->type = esrc->type;
}

static void
copy_listelem_elem(MincListElem *edest, MincListElem *esrc)
{
   DPRINT2("copy_listelem_elem(%p, %p)\n", edest, esrc);
   copy_value(&edest->val, edest->type, &esrc->val, esrc->type);
   edest->type = esrc->type;
}

/* This recursive function frees space. */
void
free_tree(Tree tp)
{
   if (tp == NULL)
      return;

#ifdef DEBUG_MEMORY
	DPRINT2("entering free_tree(%p) (%s)\n", tp, printNodeKind(tp->kind));
#endif

   switch (tp->kind) {
      case NodeZero:
         break;
      case NodeSeq:
      case NodeFuncSeq:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeStore:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeList:
         free_tree(tp->u.child[0]);
         break;
      case NodeListElem:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeEmptyListElem:
         break;
      case NodeSubscriptRead:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeSubscriptWrite:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         free_tree(tp->u.child[2]);
         break;
      case NodeOpAssign:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeName:
      case NodeAutoName:
         break;
	   case NodeDecl:
		   break;
	   case NodeFuncDecl:
		   break;
	   case NodeBlock:
		   free_tree(tp->u.child[0]);
		   break;
     case NodeString:
         break;
      case NodeConstf:
         break;
      case NodeFuncDef:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         free_tree(tp->u.child[2]);
        break;
      case NodeArgListElem:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeArgList:
         free_tree(tp->u.child[0]);
         break;
      case NodeRet:
         free_tree(tp->u.child[0]);
         break;
      case NodeCall:
         free_tree(tp->u.child[0]);
         break;
      case NodeNot:
         free_tree(tp->u.child[0]);
         break;
      case NodeAnd:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeRelation:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeOperator:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeUnaryOperator:
         free_tree(tp->u.child[0]);
         break;
      case NodeOr:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeIf:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeIfElse:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         free_tree(tp->u.child[2]);
         break;
      case NodeWhile:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         break;
      case NodeNoop:
         break;
      case NodeFor:
         free_tree(tp->u.child[0]);
         free_tree(tp->u.child[1]);
         free_tree(tp->u.child[2]);
         free_tree(tp->u.child[3]);
         break;
	   default:
		   minc_internal_error("free_tree: tried to destroy invalid node '%s'", printNodeKind(tp->kind));
		   break;
   } /* switch kind */

   if (tp->type == MincHandleType) {
      unref_handle(tp->v.handle);
   }
   else if (tp->type == MincListType) {
      unref_value_list(&tp->v);
   }
   tp->type = MincVoidType;		// To prevent double-free
   efree(tp);   /* actually free space */
#ifdef DEBUG_MEMORY
	--numTrees;
   DPRINT1("[%d trees left]\n", numTrees);
#endif
//   DPRINT1("leaving free_tree(%p)\n", tp);
}

void print_tree(Tree tp)
{
#ifdef DEBUG
	rtcmix_print("Tree %p: %s type: %d\n", tp, printNodeKind(tp->kind), tp->type);
	if (tp->kind == NodeName) {
		rtcmix_print("Symbol:\n");
		print_symbol(tp->u.symbol);
	}
	else if (tp->type == MincVoidType && tp->u.child[0] != NULL) {
		rtcmix_print("Child 0:\n");
		print_tree(tp->u.child[0]);
	}
#endif
}

static const char *MincTypeName(MincDataType type)
{
	switch (type) {
		case MincVoidType:
			return "void";
		case MincFloatType:
			return "float";
		case MincStringType:
			return "string";
		case MincHandleType:
			return "handle";
		case MincListType:
			return "list";
	}
}

void print_symbol(struct symbol * s)
{
#ifdef DEBUG
	rtcmix_print("Symbol %p: '%s' scope: %d type: %d\n", s, s->name, s->scope, s->type);
#endif
}

void
clear_elem(MincListElem *elem)
{
	if (elem->type == MincListType) {
	   DPRINT1("clear_elem(%p)\n", elem);
       unref_value_list(&elem->val);
	}
	else if (elem->type == MincHandleType) {
	   DPRINT1("clear_elem(%p)\n", elem);
	   unref_handle(elem->val.handle);
	}
}


void
unref_value_list(MincValue *value)
{
#ifdef DEBUG_MEMORY
   DPRINT3("unref_value_list(%p) [%d -> %d]\n", value->list, value->list->refcount, value->list->refcount-1);
#endif
   assert(value->list->refcount > 0);
   if (--value->list->refcount == 0) {
      if (value->list->data != NULL) {
		 int e;
		 DPRINT1("\tfreeing MincList data %p...\n", value->list->data);
		 for (e = 0; e < value->list->len; ++e) {
			 MincListElem *elem = &value->list->data[e];
			 clear_elem(elem);
		 }
		 efree(value->list->data);
		 value->list->data = NULL;
		 DPRINT("\tdone\n");
      }
	  DPRINT1("\tfreeing MincList %p\n", value->list);
	  efree(value->list);
   }
}

