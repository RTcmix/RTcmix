/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "minc_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Option.h>
#include <ugens.h>
#include <rtdefs.h>

/* Minc builtin functions, for use only in Minc scripts.
   To add a builtin function, make an entry for it in the function ptr array
   below, make a prototype for it, and define the function in this file.
   Follow the model of the existing builtins at the bottom of the file.

   John Gibson, 1/24/2004
*/

/* builtin function prototypes */
static MincFloat _minc_print(const MincValue args[], const int nargs);
static MincFloat _minc_printf(const MincValue args[], const int nargs);
static MincFloat _minc_len(const MincValue args[], const int nargs);
static MincFloat _minc_interp(const MincValue args[], const int nargs);
static MincFloat _minc_index(const MincValue args[], const int nargs);
static MincString _minc_type(const MincValue args[], const int nargs);
static MincString _minc_tostring(const MincValue args[], const int nargs);

/* other prototypes */
static int _find_builtin(const char *funcname);
static void _do_print(const MincValue args[], const int nargs);
static MincString _make_type_string(const MincDataType type);


/* list of builtin functions, searched by _find_builtin */
static struct _builtins {
   const char *label;
   MincFloat (*number_return)(const MincValue *, const int); /* func name for those returning MincFloat */
   MincString (*string_return)(const MincValue *, const int);   /* func name for those returning char * */
} builtin_funcs[] = {
   { "print",     _minc_print,   NULL },
   { "printf",    _minc_printf,  NULL },
   { "len",       _minc_len,     NULL },
   { "interp",    _minc_interp,  NULL },
   { "index",     _minc_index,   NULL },
   { "type",      NULL,          _minc_type },
   { "tostring",  NULL,          _minc_tostring },
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
call_builtin_function(const char *funcname, const MincValue arglist[],
   const int nargs, MincValue *retval)
{
   int index = _find_builtin(funcname);
   if (index < 0)
      return FUNCTION_NOT_FOUND;
   if (builtin_funcs[index].number_return) {
      *retval = (MincFloat) (*(builtin_funcs[index].number_return))
                                                         (arglist, nargs);
   }
   else if (builtin_funcs[index].string_return) {
      *retval = (MincString) (*(builtin_funcs[index].string_return))
                                                         (arglist, nargs);
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
_do_print(const MincValue args[], const int nargs)
{
   int i, last_arg;

   last_arg = nargs - 1;
   for (i = 0; i < nargs; i++) {
      switch (args[i].dataType()) {
         case MincFloatType:
            if (i == last_arg)
               RTPrintfCat("%.12g", (MincFloat)args[i]);
            else
               RTPrintfCat("%.12g, ", (MincFloat)args[i]);
            break;
         case MincStringType:
            if (i == last_arg)
               RTPrintfCat("\"%s\"", (MincString)args[i]);
            else
               RTPrintfCat("\"%s\", ", (MincString)args[i]);
            break;
         case MincHandleType:
            if (i == last_arg)
               RTPrintfCat("Handle:%p", (MincHandle)args[i]);
            else
               RTPrintfCat("Handle:%p, ", (MincHandle)args[i]);
            break;
         case MincListType:
		  {
			  MincList *list = (MincList *)args[i];
			if (list != NULL) {
				RTPrintfCat("[");
				_do_print(list->data, list->len);
				if (i == last_arg)
					RTPrintfCat("]");
				else
					RTPrintfCat("], ");
			}
			else {
				if (i == last_arg)
					RTPrintfCat("NULL");
				else
					RTPrintfCat("NULL, ");
			}
		  }
            break;
         case MincVoidType:
			  if (i == last_arg)
				  RTPrintfCat("(void)");
			  else
				  RTPrintfCat("(void), ");
           break;
      }
   }
}


/* ----------------------------------------------------------------- print -- */
MincFloat
_minc_print(const MincValue args[], const int nargs)
{
   if (get_print_option() < MMP_PRINTS) return 0.0;

   _do_print(args, nargs);
   RTPrintf("\n");
   return 0.0;
}


/* ---------------------------------------------------------------- printf -- */
/* A primitive version of printf, supporting some Minc-specific things.
   Conversion specifiers are:

      d      print float as integer
      f      print float
      l      print list
      s      print string
      t      print type of object
      z      print using the style appropriate for the type

   Escapes are \n for newline, \t for tab.  You must put the newline in
   if you want one.

   Example:

      a = 1.2345
      b = { -2, -1, 0, 1, 2, 99.99 }
      c = "boo!"
      printf("a=%d, a=%f, b=%l, c=%s, type of c: %t\n", a, a, b, c, c)

   This will print...

      a=1, a=1.2345, b=[-2, -1, 0, 1, 2, 99.99], c=boo!, type of c: string
*/

#if defined(EMBEDDED)
MincFloat
_minc_printf(const MincValue args[], const int nargs)
{
   int n;
   const char *p;
	int nchars;

	if (get_print_option() < MMP_PRINTS) return 0.0;

   if (args[0].dataType() != MincStringType) {
      minc_warn("printf: first argument must be format string");
      goto err;
   }

   n = 1;
   p = (MincString) args[0];
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
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%d", (int) (MincFloat)args[n]);
                  break;
               case 'f':      /* print float object */
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%.12g", (MincFloat)args[n]);
                  break;
               case 'l':      /* print list object */
                  if (args[n].dataType() != MincListType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%s", "[");
                  set_mm_print_ptr(nchars);
				  _do_print(((MincList *)args[n])->data, ((MincList *)args[n])->len);
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%s", "]");
                  set_mm_print_ptr(nchars);
                  break;
               case 's':      /* print string object */
                  if (args[n].dataType() != MincStringType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%s", (MincString)args[n]);
                  break;
               case 't':      /* print type of object */
                  {
                     char *tstr = (char *) _make_type_string(args[n].dataType());
	                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%s", tstr);
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
						nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "\n");
                  break;
               case 't':
						nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "\t");
                  break;
//FIXME: currently, minc.l can't handle escaped quotes in strings
               case '\'':
						nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "'");
                  break;
               case '"':
						nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "\"");
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
			nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%.1s", p);
            p++;
            break;
      }
		set_mm_print_ptr(nchars);
   }
	set_mm_print_ptr(1);
   return 0.0;
err:
	nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "\n");
	set_mm_print_ptr(nchars+1);
   return -1.0;
}

#else
MincFloat
_minc_printf(const MincValue args[], const int nargs)
{
   int n;
   const char *p;

   if (get_print_option() < MMP_PRINTS) return 0.0;

   if (args[0].dataType() != MincStringType) {
      minc_warn("printf: first argument must be format string");
      goto err;
   }

   n = 1;
   p = (MincString) args[0];
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
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  RTPrintfCat("%d", (int) (MincFloat)args[n]);
                  break;
               case 'f':      /* print float object */
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  RTPrintfCat("%.12g", (MincFloat)args[n]);
                  break;
               case 'l':      /* print list object */
                  if (args[n].dataType() != MincListType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  RTPrintfCat("[");
                  _do_print(((MincList *)args[n])->data, ((MincList *)args[n])->len);
                  RTPrintfCat("]");
                  break;
               case 's':      /* print string object */
                  if (args[n].dataType() != MincStringType) {
                     minc_warn("printf: wrong argument type for format");
                     goto err;
                  }
                  RTPrintfCat("%s", (MincString)args[n]);
                  break;
               case 't':      /* print type of object */
                  {
                     char *tstr = (char *) _make_type_string(args[n].dataType() );
                     RTPrintfCat("%s", tstr);
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
                  RTPrintfCat("\n");
                  break;
               case 't':
                  RTPrintfCat("\t");
                  break;
//FIXME: currently, minc.l can't handle escaped quotes in strings
               case '\'':
                  RTPrintfCat("'");
                  break;
               case '"':
                  RTPrintfCat("\"");
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
            RTPrintfCat("%c", *p);
            p++;
            break;
      }
   }
   return 0.0;
err:
   RTPrintf("\n");
//   fflush(stdout);
   return -1.0;
}
#endif // EMBEDDED


/* ------------------------------------------------------------------- len -- */
/* Print the length of the argument.  This is useful for getting the number
   of items in a list or the number of characters in a string.
*/
MincFloat
_minc_len(const MincValue args[], const int nargs)
{
   unsigned long len = 0;

   if (nargs != 1)
      minc_warn("len: must have one argument");
   else {
      switch (args[0].dataType() ) {
         case MincFloatType:
            len = 1;
            break;
         case MincStringType:
            len = strlen((MincString)args[0]);
            break;
         case MincHandleType:
            /* NB: To get length of a table, call tablelen(handle) */
            len = 1;
            break;
         case MincListType:
            len = ((MincList *)args[0])->len;
            break;
         default:
            minc_warn("len: invalid argument");
            break;
      }
   }
   return (MincFloat) len;
}

static int min(int x, int y) { return (x <= y) ? x : y; }

/* ------------------------------------------------------------------- interp -- */
/* Return an interpolated numeric value from a list based on a fractional
   "distance" through the list.
 */
MincFloat
_minc_interp(const MincValue args[], const int nargs)
{
	MincFloat outValue = -1;
	if (nargs != 2)
		minc_warn("interp: must have two arguments (list, fraction)");
	else {
		assert(args[1].dataType() == MincFloatType);	// must pass a float as fractional value
		if (args[0].dataType() != MincListType) {
			minc_warn("interp: first argument must be a list");
			return -1.0;
		}
		MincValue *data = ((MincList*)args[0])->data;
		int len = ((MincList*)args[0])->len;
		// Deal with degenerate cases
		if (len == 0)
			return 0.0;
		else if (len == 1)
			return (MincFloat)data[0];
		float fraction = (MincFloat)args[1];
		fraction = (fraction < 0.0) ? 0.0 : (fraction > 1.0) ? 1.0 : fraction;
		int lowIndex = (int)((len - 1) * fraction);
		int highIndex = min(len - 1, lowIndex + 1);
		if (data[lowIndex].dataType() != MincFloatType || data[highIndex].dataType() != MincFloatType) {
			minc_warn("interp: list elements to interpolate must both be floats");
			return -1;
		}
		outValue = (MincFloat)data[lowIndex] + fraction * ((MincFloat)data[highIndex] - (MincFloat)data[lowIndex]);
	}
	return outValue;
}

/* ----------------------------------------------------------------- index -- */
/* Given an item (float, string or handle), return the index of the item within
   the given list, or -1 if the item is not in the list.  Example:

      list = {1, 2, "three", 4}
      id = index(list, 2)

   <id> equals 1 after this call.
*/
MincFloat
_minc_index(const MincValue args[], const int nargs)
{
   int i, len, index = -1;
   MincDataType argtype;
   MincValue *data;

   if (nargs != 2) {
      minc_warn("index: must have two arguments (list, item_to_find)");
      return -1.0;
   }
   if (args[0].dataType() != MincListType) {
      minc_warn("index: first argument must be a list");
      return -1.0;
   }
   argtype = args[1].dataType() ;
   assert(argtype == MincFloatType || argtype == MincStringType
            || argtype == MincHandleType || argtype == MincListType);

   len = ((MincList *)args[0])->len;
   data = ((MincList *)args[0])->data;

   for (i = 0; i < len; i++) {
      if (data[i].dataType() == argtype) {
         if (argtype == MincFloatType) {
            if ((MincFloat)data[i] == (MincFloat)args[1]) {
               index = i;
               break;
            }
         }
         else if (argtype == MincStringType) {
            if (strcmp((MincString)data[i], (MincString)args[1]) == 0) {
               index = i;
               break;
            }
         }
//FIXME: should this recurse and match entire list contents??
         else if (argtype == MincListType) {
            if ((MincList*)data[i] == (MincList*)args[1]) {
               index = i;
               break;
            }
         }
         else if (argtype == MincHandleType) {
			 if ((MincHandle)data[i] == (MincHandle)args[1]) {
               index = i;
               break;
            }
         }
      }
   }

   return (MincFloat) index;
}


/* ------------------------------------------------------------------ type -- */
/* Print the object type of the argument: float, string, handle, list.
*/
MincString
_minc_type(const MincValue args[], const int nargs)
{
   if (nargs != 1) {
      minc_warn("type: must have one argument");
      return NULL;
   }
   return _make_type_string(args[0].dataType() );
}

/* ------------------------------------------------------------------ tostring -- */
/* Return the passed in (double) argument as a string type.
 */
MincString
_minc_tostring(const MincValue args[], const int nargs)
{
	if (nargs != 1) {
		minc_warn("tostring: must have one argument");
		return NULL;
	}
	if (args[0].dataType() != MincFloatType) {
		minc_warn("tostring: argument must be float type");
		return NULL;
	}
	const char *convertedString = DOUBLE_TO_STRING((MincString)args[0]);
	return strdup(convertedString);
}

