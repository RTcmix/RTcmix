/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ugens.h>      // for die, warn
#include <rtcmix_types.h>
#include <ug_intro.h>
#include <string.h>
#include <Option.h>

#define WARN_DUPLICATES

typedef struct _func {
   struct _func *next;
   union {
      double (*legacy_return) (float *, int, double *);
      double (*number_return) (const Arg[], int);
      char   *(*string_return) (const Arg[], int);
      Handle (*handle_return) (const Arg[], int);
   } func_ptr;
   RTcmixType return_type;
   const char  *func_label;
   int   legacy;        /* 1 if calling using old signature (w/ p[], pp[]) */
} RTcmixFunction;

/* --------------------------------------------------------------- addfunc -- */
/* Place a function into the table we search when handed a function name
   from the parser.  addfunc is called only from the UG_INTRO* macros.

   This is as cumbersome as it is due to the problem of handling function
   pointers having various return types.  Using the UG_INTRO macros allows
   us to declare the functions in line before passing them to addfunc.

   -JGG, 22 Jan, 2004
*/
void
RTcmix::addfunc(
   const char     	*func_label,            /* name of function exposed to script */
   LegacyFunction 	func_ptr_legacy,
   NumberFunction	func_ptr_number,
   StringFunction 	func_ptr_string,
   HandleFunction 	func_ptr_handle,
   int			    return_type,            /* return type of function */
   int    			legacy)                 /* use old function signature */
{
   RTcmixFunction *cur_node, *this_node;

   /* Create and initialize new list node. */
   this_node = new RTcmixFunction;
   if (this_node == NULL) {
      die("addfunc", "no memory for table of functions");
      return;
   }

   this_node->next = NULL;
   switch (return_type) {
      case DoubleType:
	  	if (legacy)
			this_node->func_ptr.legacy_return = func_ptr_legacy;
		else
			this_node->func_ptr.number_return = func_ptr_number;
         break;
      case StringType:
         this_node->func_ptr.string_return = func_ptr_string;
         break;
      case HandleType:
         this_node->func_ptr.handle_return = func_ptr_handle;
         break;
      default:
         die("addfunc", "invalid function return type");
         return;
   }
   this_node->return_type = (RTcmixType) return_type;
   this_node->func_label = func_label;
   this_node->legacy = legacy;

   /* Place new node at tail of list.  Warn if this function name is already
      in list.
   */
   if (_func_list == NULL) {
      _func_list = this_node;
      return;
   }
   cur_node = _func_list;
   for ( ; cur_node->next; cur_node = cur_node->next) {
#ifdef WARN_DUPLICATES
      if (strcmp(cur_node->func_label, this_node->func_label) == 0) {
         warn("addfunc", "function already introduced");
         return;
      }
#endif
   }
   cur_node->next = this_node;
} 


/* ------------------------------------------------------------- findfunc -- */
static RTcmixFunction *
findfunc(RTcmixFunction *func_list, const char *func_label)
{
   RTcmixFunction *cur_node;

   for (cur_node = func_list; cur_node; cur_node = cur_node->next) {
      if (strcmp(cur_node->func_label, func_label) == 0) {
         return cur_node;
         break;
      }
   }
   return NULL;
}


/* ------------------------------------------------------------ _printargs -- */
static void
_printargs(const char *funcname, const Arg arglist[], const int nargs)
{
   int i;
   Arg arg;

   if (Option::print()) {
      printf("============================\n");
      printf("%s:  ", funcname);
      for (i = 0; i < nargs; i++) {
         arglist[i].printInline(stdout);
      }
      putchar('\n');
      fflush(stdout);
   }
}


/* ------------------------------------------------------------- checkfunc -- */
int
RTcmix::checkfunc(const char *funcname, const Arg arglist[], const int nargs,
		          Arg *retval)
{
   RTcmixFunction *func;

   func = ::findfunc(_func_list, funcname);
   if (func == NULL)
      return -1;

   /* function found, so call it */

   ::_printargs(funcname, arglist, nargs);

   switch (func->return_type) {
   case DoubleType:
      if (func->legacy) {
         /* for old (float p[], int nargs, double pp[]) signature */
         #include <maxdispargs.h>
         float p[MAXDISPARGS];
         double pp[MAXDISPARGS];
         for (int i = 0; i < nargs; i++) {
			const Arg &theArg = arglist[i];
            p[i] = (float) theArg;
			switch (theArg.type()) {
            case DoubleType:
               pp[i] = (double) theArg;
			   break;
            case StringType:
               pp[i] = (double) (int) ((const char *) theArg);
			   break;
            default:
               die("%s: arguments must be numbers or strings.", funcname);
               return -1;
            }
         }
         /* some functions rely on zero contents of args > nargs */
         for (int i = nargs; i < MAXDISPARGS; i++) {
            p[i] = 0.0;
            pp[i] = 0.0;
         }
         *retval = (double) (*(func->func_ptr.legacy_return))
                                                      (p, nargs, pp);
      }
      else
         *retval = (double) (*(func->func_ptr.number_return))
                                                      (arglist, nargs);
      break;
   case HandleType:
      *retval = (Handle) (*(func->func_ptr.handle_return))
                                                      (arglist, nargs);
      break;
   case StringType:
      *retval = (const char *) (*(func->func_ptr.string_return))
                                                      (arglist, nargs);
      break;
   default:
      break;
   }

   return 0;
}

// Wrappers for UG_INTRO() use.

extern "C" {
void addfunc(
   const char     	*func_label,            /* name of function exposed to script */
   LegacyFunction 	func_ptr_legacy,
   NumberFunction	func_ptr_number,
   StringFunction 	func_ptr_string,
   HandleFunction 	func_ptr_handle,
   RTcmixType		return_type,            /* return type of function */
   int    			legacy)                 /* use old function signature */
{
	RTcmix::addfunc(func_label, func_ptr_legacy, func_ptr_number,
				    func_ptr_string, func_ptr_handle, return_type, legacy);
}
};

void
addLegacyfunc(const char *label, double (*func_ptr)(float *, int, double *))
{
	RTcmix::addfunc(label, func_ptr, NULL, NULL, NULL, DoubleType, 1);
}

