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

#include "minc_internal.h"
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
static MincListElem *list;
static int list_len;
static MincListElem *list_stack[MAXSTACK];
static int list_len_stack[MAXSTACK];
static int list_stack_ptr;

/* prototypes for local functions */
static int cmp(MincFloat f1, MincFloat f2);
static Tree node(OpKind op, NodeKind kind);
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


static Tree
node(OpKind op, NodeKind kind)
{
   Tree tp;

   tp = (Tree) emalloc(sizeof(struct tree));
   tp->op = op;
   tp->kind = kind;
   tp->type = MincVoidType;
   tp->u.child[0] = NULL;    /* these clear entire <u> union */
   tp->u.child[1] = NULL;
   tp->u.child[2] = NULL;
   tp->u.child[3] = NULL;
#if 1 // makes debugging less painful to clear these
   tp->v.list.len = 0;
   tp->v.list.data = NULL;
   tp->funcname = NULL;
#endif

   return tp;
}


Tree
tnoop()
{
   Tree tp = node(OpFree, NodeNoop);

   DPRINT1("tnoop => %p\n", tp);
   return tp;
}


Tree
tseq(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeSeq);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tseq (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


Tree
top(OpKind op, Tree e1, Tree e2)
{
   Tree tp = node(op, NodeOperator);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("top (%d, %p, %p) => %p\n", op, e1, e2, tp);
   return tp;
}


Tree
tunop(OpKind op, Tree e1)
{
   Tree tp = node(op, NodeUnaryOperator);
   tp->u.child[0] = e1;

   DPRINT3("tunop (%d, %p) => %p\n", op, e1, tp);
   return tp;
}


/* store a value into a variable */
Tree
tstore(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeStore);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tstore (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


/* like tstore, but modify value before storing into variable */
Tree
topassign(Tree e1, Tree e2, OpKind op)
{
   Tree tp = node(op, NodeOpAssign);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("topassign, op=%d (%p, %p) => %p\n", op, e1, e2, tp);
   return tp;
}


/* converts symbol table entry into tree
   or initialize tree node to a symbol entry
*/
Tree
tname(Symbol *symbol)
{
   Tree tp = node(OpFree, NodeName);
   tp->u.symbol = symbol;

   DPRINT2("tname ('%s') => %p\n", symbol->name, tp);
   return tp;
}


Tree
tstring(char *str)
{
   Tree tp = node(OpFree, NodeString);
   tp->u.string = str;

   DPRINT2("tstring ('%s') => %p\n", str, tp);
   return tp;
}


Tree
tconstf(MincFloat num)
{
   Tree tp = node(OpFree, NodeConstf);
   tp->u.number = num;

   DPRINT2("tconstf (%f) => %p\n", num, tp);
   return tp;
}


Tree
tcall(Tree args, char *funcname)
{
   Tree tp = node(OpFree, NodeCall);
   tp->funcname = funcname;
   tp->u.child[0] = args;

   DPRINT3("tcall ('%s', %p) => %p\n", funcname, args, tp);
   return tp;
}


Tree
tcand(Tree test1, Tree test2)
{
   Tree tp = node(OpFree, NodeAnd);
   tp->u.child[0] = test1;
   tp->u.child[1] = test2;

   DPRINT3("tcand (%p, %p) => %p\n", test1, test2, tp);
   return tp;
}


Tree
tcor(Tree test1, Tree test2)
{
   Tree tp = node(OpFree, NodeOr);
   tp->u.child[0] = test1;
   tp->u.child[1] = test2;

   DPRINT3("tcor (%p, %p) => %p\n", test1, test2, tp);
   return tp;
}


Tree
tnot(Tree test1)
{
   Tree tp = node(OpFree, NodeNot);
   tp->u.child[0] = test1;

   DPRINT2("tnot (%p) => %p\n", test1, tp);
   return tp;
}


Tree
trel(OpKind op, Tree e1, Tree e2)
{
   Tree tp = node(op, NodeRelation);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT4("trel (%d, %p, %p) => %p\n", op, e1, e2, tp);
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
   tp->u.child[0] = e1;

   DPRINT2("tlist (%p) => %p\n", e1, tp);
   return tp;
}


Tree
tlistelem(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeListElem);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tlistelem (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


Tree
temptylistelem()
{
   Tree tp = node(OpFree, NodeEmptyListElem);

   DPRINT1("temptylistelem => %p\n", tp);
   return tp;
}


Tree
tsubscriptread(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeSubscriptRead);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tsubscriptread (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


Tree
tsubscriptwrite(Tree e1, Tree e2, Tree e3)
{
   Tree tp = node(OpFree, NodeSubscriptWrite);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;

   DPRINT4("tsubscriptwrite (%p, %p, %p) => %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
tif(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeIf);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("tif (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


Tree
tifelse(Tree e1, Tree e2, Tree e3)
{
   Tree tp = node(OpFree, NodeIfElse);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;

   DPRINT4("tifelse (%p, %p, %p) => %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
tfor(Tree e1, Tree e2, Tree e3, Tree e4)
{
   Tree tp = node(OpFree, NodeFor);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;
   tp->u.child[2] = e3;
   tp->u.child[3] = e4;

   DPRINT4("tfor (%p, %p, %p, <e4>) => %p\n", e1, e2, e3, tp);
   return tp;
}


Tree
twhile(Tree e1, Tree e2)
{
   Tree tp = node(OpFree, NodeWhile);
   tp->u.child[0] = e1;
   tp->u.child[1] = e2;

   DPRINT3("twhile (%p, %p) => %p\n", e1, e2, tp);
   return tp;
}


/* ---------------------------------------------------------- do_op_string -- */
static void
do_op_string(Tree tp, const char *str1, const char *str2, OpKind op)
{
   char *s;
   int   len;

   switch (op) {
      case OpPlus:   /* concatenate */
         len = (strlen(str1) + strlen(str2)) + 1;
         s = (char *) emalloc(sizeof(char) * len);
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
      default:
         break;
   }
   tp->type = MincStringType;
}


/* ------------------------------------------------------------- do_op_num -- */
static void
do_op_num(Tree tp, const MincFloat val1, const MincFloat val2, OpKind op)
{
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
         break;
   }
   tp->type = MincFloatType;
}


/* ---------------------------------------------------- do_op_list_iterate -- */
/* Iterate over <child>'s list, performing the operation specified by <op>,
   using the scalar <val>, for each list element.  Store the result into a
   new list for <tp>, so that child's list is unchanged.
*/
static void
do_op_list_iterate(Tree tp, Tree child, const MincFloat val, const OpKind op)
{
   unsigned int i;
   const unsigned int len = child->v.list.len;
   MincListElem *src = child->v.list.data;
   MincListElem *dest = (MincListElem *) emalloc(sizeof(MincListElem) * len);
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
         minc_internal_error("invalid list operation");
         break;
   }
   tp->type = MincListType;
   tp->v.list = child->v.list;
   tp->v.list.data = dest;
}


/* --------------------------------------------------------- exct_operator -- */
static void
exct_operator(Tree tp, OpKind op)
{
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
               minc_warn("can't operate on a string");
               break;
            case MincHandleType:
               minc_warn("can't operate on a handle");
               break;
            case MincListType:
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
               minc_warn("can't operate on a string and a number");
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
         minc_warn("can't operate on a handle");
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
   Tree tp0 = tp->u.child[0];
   Tree tp1 = exct(tp->u.child[1]);

   if (tp0->u.symbol->type != MincFloatType || tp1->type != MincFloatType) {
      minc_warn("can only use '%c=' with numbers",
                           op == OpPlus ? '+' : (op == OpMinus ? '-'
                                              : (op == OpMul ? '*' : '/')));
//FIXME: Is this correct?
      memcpy(&tp->v, &tp0->u.symbol->v, sizeof(MincValue));
      tp->type = tp0->type;
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
   exct(tp->u.child[1]);
   if (tp->u.child[1]->type == MincFloatType) {
      if (tp->u.child[0]->u.symbol->type == MincListType) {
         MincListElem elem;
         MincFloat fltindex = tp->u.child[1]->v.number;
         unsigned int index = (unsigned int) fltindex;
         unsigned int index2 = index + 1;
//FIXME: do linear interp for number items
         MincListElem *lst = tp->u.child[0]->u.symbol->v.list.data;
         unsigned int len = tp->u.child[0]->u.symbol->v.list.len;
         if (len < 1)
            minc_die("attempt to index an empty list");
         if (fltindex == -1.0)   /* means last element */
            index = len - 1;
         else if (fltindex > (MincFloat) (len - 1)) {
            minc_warn("attempt to index past the end of list ... "
                                                "returning last element");
            index = len - 1;
         }
         elem = lst[index];
         tp->type = elem.type;
         memcpy(&tp->v, &elem.val, sizeof(MincValue));
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
   exct(tp->u.child[1]);         /* index */
   exct(tp->u.child[2]);         /* expression to store */
   if (tp->u.child[1]->type == MincFloatType) {
      if (tp->u.child[0]->u.symbol->type == MincListType) {
         unsigned int len;
         MincListElem *lst, elem;
         MincFloat fltindex = tp->u.child[1]->v.number;
         unsigned int index = (unsigned int) fltindex;
         if (fltindex - (MincFloat) index > 0.0)
            minc_warn("list index must be integer ... correcting");
         lst = tp->u.child[0]->u.symbol->v.list.data;
         len = tp->u.child[0]->u.symbol->v.list.len;
         assert(len >= 0);    /* NB: okay to have zero-length list */
         if (index == -1)     /* means last element */
            index = len > 0 ? len - 1 : 0;
         else if (index >= len) {
            /* realloc list and init new slots */
            unsigned int i, newslots, oldlen = len;
            newslots = len > 0 ? (index - (len - 1)) : index + 1;
            len += newslots;
            lst = (MincListElem *) realloc(lst, sizeof(MincListElem) * len);
            tp->u.child[0]->u.symbol->v.list.data = lst;
            tp->u.child[0]->u.symbol->v.list.len = len;
            for (i = oldlen; i < len; i++) {
               lst[i].type = MincFloatType;
               lst[i].val.number = 0.0;
            }
         }
         lst[index].type = tp->u.child[2]->type;
         memcpy(&lst[index].val, &tp->u.child[2]->v, sizeof(MincValue));
         tp->type = tp->u.child[2]->type;
         memcpy(&tp->v, &tp->u.child[2]->v, sizeof(MincValue));
      }
      else
         minc_die("attempt to index a variable that's not a list");
   }
   else
      minc_die("list index must be a number");
}


/* ------------------------------------------------------------------ exct -- */
/* This recursive function interprets the intermediate code.
*/
Tree
exct(Tree tp)
{
   if (tp == NULL)
      return NULL;

   switch (tp->kind) {
      case NodeConstf:
         DPRINT1("exct (enter NodeConstf, tp=%p)\n", tp);
         tp->type = MincFloatType;
         tp->v.number = tp->u.number;
         DPRINT1("exct (exit NodeConstf, tp=%p)\n", tp);
         break;
      case NodeString:
         DPRINT1("exct (enter NodeString, tp=%p)\n", tp);
         tp->type = MincStringType;
         tp->v.string = tp->u.string;
         DPRINT1("exct (exit NodeString, tp=%p)\n", tp);
         break;
      case NodeName:
         DPRINT1("exct (enter NodeName, tp=%p)\n", tp);
         /* assign what's in the symbol into tree's value field */
         tp->type = tp->u.symbol->type;
         memcpy(&tp->v, &tp->u.symbol->v, sizeof(MincValue));
         DPRINT1("exct (exit NodeName, tp=%p)\n", tp);
         break;
      case NodeListElem:
         DPRINT1("exct (enter NodeListElem, tp=%p)\n", tp);
         exct(tp->u.child[0]);
         if (list_len == MAXDISPARGS)
            minc_die("exceeded maximum number of items for a list");
         {
            Tree tmp = exct(tp->u.child[1]);
            /* Copy entire MincValue union from expr to tp and to stack. */
            memcpy(&tp->v, &tmp->v, sizeof(MincValue));
            tp->type = tmp->type;
            memcpy(&list[list_len].val, &tmp->v, sizeof(MincValue));
            list[list_len].type = tmp->type;
            list_len++;
         }
         DPRINT1("exct (exit NodeListElem, tp=%p)\n", tp);
         break;
      case NodeEmptyListElem:
         DPRINT1("exct (enter NodeEmptyListElem, tp=%p)\n", tp);
         /* do nothing */
         DPRINT("exct (exit NodeEmptyListElem)\n");
         break;
      case NodeList:
         DPRINT1("exct (enter NodeList, tp=%p)\n", tp);
         push_list();
         exct(tp->u.child[0]);     /* NB: increments list_len */
         {
            MincListElem *data;
            data = (MincListElem *) emalloc(sizeof(MincListElem) * list_len);
            tp->type = MincListType;
            tp->v.list.data = data;
            tp->v.list.len = list_len;
            memcpy(data, list, sizeof(MincListElem) * list_len);
         }
         pop_list();
         DPRINT1("exct (exit NodeList, tp=%p)\n", tp);
         break;
      case NodeSubscriptRead:
         DPRINT1("exct (enter NodeSubscriptRead, tp=%p)\n", tp);
         exct_subscript_read(tp);
         DPRINT1("exct (exit NodeSubscriptRead, tp=%p)\n", tp);
         break;
      case NodeSubscriptWrite:
         DPRINT1("exct (enter NodeSubscriptWrite, tp=%p)\n", tp);
         exct_subscript_write(tp);
         DPRINT1("exct (exit NodeSubscriptWrite, tp=%p)\n", tp);
         break;
      case NodeCall:
         DPRINT1("exct (enter NodeCall, tp=%p)\n", tp);
         push_list();
         exct(tp->u.child[0]);
         {
            MincListElem retval;
            int result = call_builtin_function(tp->funcname, list, list_len,
                                                                     &retval);
            if (result < 0)
               result = call_external_function(tp->funcname, list, list_len,
                                                                     &retval);
            tp->type = retval.type;
            memcpy(&tp->v, &retval.val, sizeof(MincValue));
         }
         pop_list();
         DPRINT1("exct (exit NodeCall, tp=%p)\n", tp);
         break;
      case NodeStore:
         DPRINT1("exct (enter NodeStore, tp=%p)\n", tp);
         /* Store value and type into sym pointed to by child[0]->u.symbol. */
         exct(tp->u.child[1]);
         /* Copy entire MincValue union from expr to id sym and to tp. */
         memcpy(&tp->u.child[0]->u.symbol->v, &tp->u.child[1]->v,
                                                         sizeof(MincValue));
         memcpy(&tp->v, &tp->u.child[1]->v, sizeof(MincValue));
         tp->type = tp->u.child[0]->u.symbol->type = tp->u.child[1]->type;
         DPRINT2("exct (exit NodeStore, tp=%p, type=%d)\n", tp, tp->type);
         break;
      case NodeOpAssign:
         DPRINT1("exct (enter NodeOpAssign, tp=%p)\n", tp);
         exct_opassign(tp, tp->op);
         DPRINT1("exct (exit NodeOpAssign, tp=%p)\n", tp);
         break;
      case NodeNot:
         DPRINT1("exct (enter NodeNot, tp=%p)\n", tp);
         tp->type = MincFloatType;
         if (cmp(0.0, exct(tp->u.child[0])->v.number) == 0)
            tp->v.number = 1.0;
         else
            tp->v.number = 0.0;
         DPRINT1("exct (exit NodeNot, tp=%p)\n", tp);
         break;
      case NodeAnd:
         DPRINT1("exct (enter NodeAnd, tp=%p)\n", tp);
         tp->type = MincFloatType;
         tp->v.number = 0.0;
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0) {
            if (cmp(0.0, exct(tp->u.child[1])->v.number) != 0) {
               tp->type = MincFloatType;
               tp->v.number = 1.0;
            }
         }
         DPRINT1("exct (exit NodeAnd, tp=%p)\n", tp);
         break;
      case NodeRelation:
         DPRINT2("exct (enter NodeRelation, tp=%p, op=%d)\n", tp, tp->op);
         exct_relation(tp);
         DPRINT2("exct (exit NodeRelation, tp=%p, op=%d)\n", tp, tp->op);
         break; /* switch NodeRelation */
      case NodeOperator:
         DPRINT2("exct (enter NodeOperator, tp=%p, op=%d)\n", tp, tp->op);
         exct_operator(tp, tp->op);
         DPRINT2("exct (exit NodeOperator, tp=%p, op=%d)\n", tp, tp->op);
         break; /* switch NodeOperator */
      case NodeUnaryOperator:
         DPRINT1("exct (enter NodeUnaryOperator, tp=%p)\n", tp);
         tp->type = MincFloatType;
         if (tp->op == OpNeg)
            tp->v.number = -exct(tp->u.child[0])->v.number;
         DPRINT1("exct (exit NodeUnaryOperator, tp=%p)\n", tp);
         break;
      case NodeOr:
         DPRINT1("exct (enter NodeOr, tp=%p)\n", tp);
         tp->type = MincFloatType;
         tp->v.number = 0.0;
         if ((cmp(0.0, exct(tp->u.child[0])->v.number) != 0) ||
             (cmp(0.0, exct(tp->u.child[1])->v.number) != 0)) {
            tp->v.number = 1.0;
         }
         DPRINT1("exct (exit NodeOr, tp=%p)\n", tp);
         break;
      case NodeIf:
         DPRINT1("exct (enter NodeIf, tp=%p)\n", tp);
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         DPRINT1("exct (exit NodeIf, tp=%p)\n", tp);
         break;
      case NodeIfElse:
         DPRINT1("exct (enter NodeIfElse, tp=%p)\n", tp);
         if (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         else
            exct(tp->u.child[2]);
         DPRINT1("exct (exit NodeIfElse, tp=%p)\n", tp);
         break;
      case NodeWhile:
         DPRINT1("exct (enter NodeWhile, tp=%p)\n", tp);
         while (cmp(0.0, exct(tp->u.child[0])->v.number) != 0)
            exct(tp->u.child[1]);
         DPRINT1("exct (exit NodeWhile, tp=%p)\n", tp);
         break;
      case NodeNoop:
         DPRINT1("exct (enter NodeNoop, tp=%p)\n", tp);
         /* do nothing */
         DPRINT("exct (exit NodeNoop)\n");
         break;
      case NodeFor:
         DPRINT1("exct (enter NodeFor, tp=%p)\n", tp);
         exct(tp->u.child[0]);         /* init */
         while (cmp(0.0, exct(tp->u.child[1])->v.number) != 0) { /* condition */
            exct(tp->u.child[3]);      /* execute block */
            exct(tp->u.child[2]);      /* prepare for next iteration */
         }
         DPRINT1("exct (exit NodeFor, tp=%p)\n", tp);
         break;
      case NodeSeq:
         DPRINT1("exct (enter NodeSeq, tp=%p)\n", tp);
         exct(tp->u.child[0]);
         exct(tp->u.child[1]);
         DPRINT1("exct (exit NodeSeq, tp=%p)\n", tp);
         break;
      default:
         minc_internal_error("exct: tried to execute invalid node");
         break;
   } /* switch kind */

   return tp;
}


static void
push_list()
{
   DPRINT("push_list\n");
   if (list_stack_ptr >= MAXSTACK)
      minc_die("stack overflow: too many nested function calls");
   list_stack[list_stack_ptr] = list;
   list_len_stack[list_stack_ptr++] = list_len;
   list = (MincListElem *) malloc(sizeof(MincListElem) * MAXDISPARGS);
   list_len = 0;
}


static void
pop_list()
{
   DPRINT("pop_list\n");
   free(list);
   if (list_stack_ptr == 0)
      minc_die("stack underflow");
   list = list_stack[--list_stack_ptr];
   list_len = list_len_stack[list_stack_ptr];
}


/* This recursive function frees space. */
void
free_tree(Tree tp)
{
   if (tp == NULL)
      return;

   switch (tp->kind) {
      case NodeZero:
         break;
      case NodeSeq:
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
         break;
      case NodeString:
         break;
      case NodeConstf:
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
   } /* switch kind */

   free(tp);   /* actually free space */
}


