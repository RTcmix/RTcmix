/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "minc_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Minc builtin functions, for use only in Minc scripts.
   To add a builtin function, make an entry for it in the function ptr array
   below, make a prototype for it, and define the function in this file.
   Follow the model of the existing builtins.

   John Gibson, 1/24/2004
*/

/* builtin function prototypes */
static MincFloat _minc_print(const MincListElem args[], const int nargs);
static MincFloat _minc_printf(const MincListElem args[], const int nargs);
static MincFloat _minc_len(const MincListElem args[], const int nargs);
static MincString _minc_type(const MincListElem args[], const int nargs);

/* other prototypes */
static int _find_builtin(const char *funcname);
static void _do_print(const MincListElem args[], const int nargs);
static MincString _make_type_string(const MincDataType type);


/* list of builtin functions, searched by _find_builtin */
static struct _builtins {
   char *label;
   MincFloat (*number_return)(); /* func name for those returning MincFloat */
   MincString (*string_return)();   /* func name for those returning char * */
} builtin_funcs[] = {
   { "print",     _minc_print,   NULL },
   { "printf",    _minc_printf,  NULL },
   { "len",       _minc_len,     NULL },
   { "type",      NULL,          _minc_type },
   { NULL,        NULL,          NULL }         /* marks end of list */
};


/* -------------------------------------- call_builtin_function and helper -- */
static int
_find_builtin(const char *funcname)
{
   int i = 0;
   while (1) {
      if (builtin_funcs[i].label == NULL)
         break;
      if (strcmp(builtin_funcs[i].label, funcname) == 0)
         return i;
      i++;
   }
   return -1;
}

int
call_builtin_function(const char *funcname, const MincListElem arglist[],
   const int nargs, MincListElem *retval)
{
   int index = _find_builtin(funcname);
   if (index < 0)
      return -1;
   if (builtin_funcs[index].number_return) {
      retval->val.number = (MincFloat) (*(builtin_funcs[index].number_return))
                                                         (arglist, nargs);
      retval->type = MincFloatType;
   }
   else if (builtin_funcs[index].string_return) {
      retval->val.string = (MincString) (*(builtin_funcs[index].string_return))
                                                         (arglist, nargs);
      retval->type = MincStringType;
   }
   return 0;
}


/* ============================================= print, printf and friends == */

/* ----------------------------------------------------- _make_type_string -- */
static MincString
_make_type_string(const MincDataType type)
{
   char *str = NULL;

   switch (type) {
      case MincVoidType:
         str = strdup("void");
         break;
      case MincFloatType:
         str = strdup("float");
         break;
      case MincStringType:
         str = strdup("string");
         break;
      case MincHandleType:
         str = strdup("handle");
         break;
      case MincListType:
         str = strdup("list");
         break;
   }
   return (MincString) str;
}


/* ------------------------------------------------------------- _do_print -- */
static void
_do_print(const MincListElem args[], const int nargs)
{
   int i, last_arg;

   last_arg = nargs - 1;
   for (i = 0; i < nargs; i++) {
      switch (args[i].type) {
         case MincFloatType:
            if (i == last_arg)
               printf("%.12g", args[i].val.number);
            else
               printf("%.12g, ", args[i].val.number);
            break;
         case MincStringType:
            if (i == last_arg)
               printf("\"%s\"", args[i].val.string);
            else
               printf("\"%s\", ", args[i].val.string);
            break;
         case MincHandleType:
            if (i == last_arg)
               printf("Handle:%p", args[i].val.handle);
            else
               printf("Handle:%p, ", args[i].val.handle);
            break;
         case MincListType:
            putchar('[');
            _do_print(args[i].val.list.data, args[i].val.list.len);
            if (i == last_arg)
               putchar(']');
            else
               printf("], ");
            break;
         default:
            break;
      }
   }
}


/* ----------------------------------------------------------------- print -- */
MincFloat
_minc_print(const MincListElem args[], const int nargs)
{
   _do_print(args, nargs);
   putchar('\n');
   fflush(stdout);
   return 0.0;
}


/* ---------------------------------------------------------------- printf -- */
/* A primitive version of printf, supporting some Minc-specific things. */
MincFloat
_minc_printf(const MincListElem args[], const int nargs)
{
   int n;
   char *p;

   if (args[0].type != MincStringType) {
      minc_warn("printf: first argument must be format string");
      goto err;
   }

   n = 1;
   p = args[0].val.string;
   while (*p) {
      switch (*p) {
         case '%':
            p++;
            if (n >= nargs) {
               minc_warn("printf: not enough arguments for format string");
               goto err;
            }
            switch (*p) {
               case 'd':      /* print float object as integer */
                  if (args[n].type != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  printf("%d", (int) args[n].val.number);
                  break;
               case 'f':      /* print float object */
                  if (args[n].type != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  printf("%.12g", args[n].val.number);
                  break;
               case 'l':      /* print list object */
                  if (args[n].type != MincListType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  putchar('[');
                  _do_print(args[n].val.list.data, args[n].val.list.len);
                  putchar(']');
                  break;
               case 's':      /* print string object */
                  if (args[n].type != MincStringType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  fputs(args[n].val.string, stdout);
                  break;
               case 't':      /* print type of object */
                  {
                     char *tstr = _make_type_string(args[n].type);
                     fputs(tstr, stdout);
                     free(tstr);
                  }
                  break;
               case 'z':      /* print as appropriate for type */
                  _do_print(&args[n], 1);
                  break;
               case '\0':
                  minc_warn("printf: premature end of format string");
                  goto err;
                  break;
               default:
                  minc_warn("printf: invalid format specifier");
                  goto err;
                  break;
            }
            n++;
            p++;
            break;
         case '\\':
            p++;
            switch (*p) {
               case 'n':
                  putchar('\n');
                  break;
               case 't':
                  putchar('\t');
                  break;
//FIXME: currently, minc.l can't handle escaped quotes in strings
               case '\'':
                  putchar('\'');
                  break;
               case '"':
                  putchar('"');
                  break;
               case '\0':
                  minc_warn("printf: premature end of format string");
                  goto err;
                  break;
               default:
                  minc_warn("printf: invalid escape character");
                  goto err;
                  break;
            }
            p++;
            break;
         default:
            putchar(*p);
            p++;
            break;
      }
   }
   return 0.0;
err:
   putchar('\n');
   fflush(stdout);
   return -1.0;
}


/* ------------------------------------------------------------------- len -- */
MincFloat
_minc_len(const MincListElem args[], const int nargs)
{
   unsigned int len = 0;

   if (nargs != 1)
      minc_warn("len: must have one argument");
   else {
      switch (args[0].type) {
         case MincFloatType:
            len = 1;
            break;
         case MincStringType:
            len = strlen(args[0].val.string);
            break;
         case MincHandleType:
            len = 1;
            break;
         case MincListType:
            len = args[0].val.list.len;
            break;
         default:
            minc_warn("len: invalid argument");
            break;
      }
   }
   return (MincFloat) len;
}


/* ------------------------------------------------------------------ type -- */
MincString
_minc_type(const MincListElem args[], const int nargs)
{
   if (nargs != 1) {
      minc_warn("type: must have one argument");
      return NULL;
   }
   return _make_type_string(args[0].type);
}


