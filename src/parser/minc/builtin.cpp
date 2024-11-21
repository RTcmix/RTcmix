/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "minc_internal.h"
#include "MincValue.h"
#include "Symbol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <RTOption.h>
#include <ugens.h>

/* Minc builtin functions, for use only in Minc scripts.
   To add a builtin function, make an entry for it in the function ptr array
   below, make a prototype for it, and define the function in this file.
   Follow the model of the existing builtins at the bottom of the file.

   John Gibson, 1/24/2004
*/

/* builtin function prototypes */
static MincFloat _minc_print(const MincValue args[], int nargs);
static MincFloat _minc_printf(const MincValue args[], int nargs);
static MincFloat _minc_print_if(const MincValue args[], int nargs);
static MincFloat _minc_error(const MincValue args[], int nargs);
static MincFloat _minc_len(const MincValue args[], int nargs);
static MincFloat _minc_interp(const MincValue args[], int nargs);
static MincFloat _minc_index(const MincValue args[], int nargs);
static MincFloat _minc_contains(const MincValue args[], int nargs);
static MincFloat _minc_remove(const MincValue args[], int nargs);
static MincFloat _minc_insert(const MincValue args[], int nargs);
static MincString _minc_type(const MincValue args[], int nargs);
static MincString _minc_tostring(const MincValue args[], int nargs);
static MincString _minc_substring(const MincValue args[], int nargs);

/* other prototypes */
static int _find_builtin(const char *funcname);
static void _do_print(const MincValue args[], int nargs);
static MincString _make_type_string(MincDataType type);


/* list of builtin functions, searched by _find_builtin */
static struct _builtins {
   const char *label;
   MincFloat (*number_return)(const MincValue *, int); /* func name for those returning MincFloat */
   MincString (*string_return)(const MincValue *, int);   /* func name for those returning char * */
} builtin_funcs[] = {
   { "print",     _minc_print,   NULL },
   { "printf",    _minc_printf,  NULL },
   { "print_if",   _minc_print_if, NULL },
   { "error",     _minc_error,   NULL },
   { "len",       _minc_len,     NULL },
   { "interp",    _minc_interp,  NULL },
   { "index",     _minc_index,   NULL },
   { "contains",  _minc_contains, NULL },
   { "remove",    _minc_remove,   NULL },
   { "insert",    _minc_insert,   NULL },
   { "type",      NULL,          _minc_type },
   { "tostring",  NULL,          _minc_tostring },
   { "substring", NULL,          _minc_substring },
   { NULL,        NULL,          NULL }         /* marks end of list */
};


/* -------------------------------------- call_builtin_function and helper -- */
static int
_find_builtin(const char *funcname)
{
   int i = 0;
   while (true) {
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
   int nargs, MincValue *retval)
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
_make_type_string(MincDataType type)
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
       case MincMapType:
           str = strdup("map");
           break;
      case MincStructType:
         str = strdup("struct");
         break;
       case MincFunctionType:
           str = strdup("function");
           break;
   }
   return (MincString) str;
}


/* ------------------------------------------------------------- _do_print -- */
static void
_do_print(const MincValue args[], int nargs)
{
   int i, last_arg;

   last_arg = nargs - 1;
   for (i = 0; i < nargs; i++) {
      const char *delimiter = (i == last_arg) ? "" : ", ";
      switch (args[i].dataType()) {
         case MincFloatType:
            RTPrintfCat("%.12g%s", (MincFloat)args[i], delimiter);
            break;
         case MincStringType:
            RTPrintfCat("\"%s\"%s", (MincString)args[i], delimiter);
            break;
         case MincHandleType:
            RTPrintfCat("Handle:%p%s", (MincHandle)args[i], delimiter);
            break;
          case MincFunctionType:
              RTPrintfCat("Function:%p%s", (MincFunction *)args[i], delimiter);
              break;
        case MincListType:
		  {
			  MincList *list = (MincList *)args[i];
			if (list != NULL) {
				RTPrintfCat("[");
                unsigned printLimit = (unsigned)RTOption::printListLimit();
                if (printLimit < list->len) {
                    _do_print(list->data, printLimit);
                    RTPrintfCat(", ...]%s", delimiter);
                }
                else {
                    _do_print(list->data, list->len);
                    RTPrintfCat("]%s", delimiter);
                }
			}
			else {
                RTPrintfCat("NULL%s", delimiter);
			}
		  }
            break;
          case MincMapType:
          {
              MincMap *mmap = (MincMap *)args[i];
              if (mmap != NULL) {
                  RTPrintfCat("[");
                  mmap->print();
                  RTPrintfCat("]%s", delimiter);
              }
              else {
                  RTPrintfCat("NULL%s", delimiter);
              }
          }
              break;
        case MincStructType:
          {
              MincStruct *theStruct = (MincStruct *)args[i];
              if (theStruct != NULL) {
                  RTPrintfCat("{ ");
                  theStruct->print();
                  RTPrintfCat(" }%s", delimiter);
              }
              else {
                  RTPrintfCat("NULL%s", delimiter);
              }
          }
              break;
         case MincVoidType:
              RTPrintfCat("(void)%s", delimiter);
              break;
      }
   }
}

// Note:  These are defined here to let them have access to the static helper routines above

void    MincStruct::print()
{
    for (Symbol *member = _memberList; member != NULL;) {
        Symbol *next = member->next;
        _do_print(&member->value(), 1);
        if (next != NULL) {
            RTPrintfCat(", ");
        }
        member = next;
    }
//    RTPrintf("\n");
}

void    MincMap::print()
{
    for (std::map<MincValue, MincValue, MincValueCmp>::iterator iter = map.begin(); iter != map.end();) {
        RTPrintfCat("key:");
        _do_print(&iter->first, 1);
        RTPrintfCat(" val:");
        _do_print(&iter->second, 1);
        ++iter;
        if (iter != map.end()) {
            RTPrintfCat(", ");
        }
    }
    RTPrintf("\n");
}


/* ----------------------------------------------------------------- print -- */
MincFloat
_minc_print(const MincValue args[], int nargs)
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

#if defined(EMBEDDED) && !FORCE_EMBEDDED_PRINTF
MincFloat
_minc_printf(const MincValue args[], int nargs)
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
                     minc_warn("printf: wrong argument type for %%f format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%d", (int) (MincFloat)args[n]);
                  break;
               case 'f':      /* print float object */
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for %%d format");
                     goto err;
                  }
                  nchars = snprintf(get_mm_print_ptr(), get_mm_print_space(), "%.12g", (MincFloat)args[n]);
                  break;
               case 'l':      /* print list object */
                  if (args[n].dataType() != MincListType) {
                     minc_warn("printf: wrong argument type for %%l format");
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
                     minc_warn("printf: wrong argument type for %%s format");
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
_minc_printf(const MincValue args[], int nargs)
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
                     minc_warn("printf: wrong argument type for %%d format");
                     goto err;
                  }
                  RTPrintfCat("%d", (int) (MincFloat)args[n]);
                  break;
               case 'f':      /* print float object */
                  if (args[n].dataType() != MincFloatType) {
                     minc_warn("printf: wrong argument type for %%f format");
                     goto err;
                  }
                  RTPrintfCat("%.12g", (MincFloat)args[n]);
                  break;
               case 'l':      /* print list object */
                  if (args[n].dataType() != MincListType) {
                     minc_warn("printf: wrong argument type for %%l format");
                     goto err;
                  }
                  RTPrintfCat("[");
                  _do_print(((MincList *)args[n])->data, ((MincList *)args[n])->len);
                  RTPrintfCat("]");
                  break;
               case 's':      /* print string object */
                  if (args[n].dataType() != MincStringType) {
                     minc_warn("printf: wrong argument type for %%s format");
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

MincFloat
_minc_print_if(const MincValue args[], int nargs)
{
    MincFloat ret = 0.0;
    if ((MincFloat)args[0] > 0.0) {
        ret = _minc_printf(&args[1], nargs-1);
    }
    return ret;
}

MincFloat
_minc_error(const MincValue args[], int nargs)
{
    MincString p = (MincString) args[0];
    minc_die("%s", p);
    return -1.0;
}

/* ------------------------------------------------------------------- len -- */
/* Print the length of the argument.  This is useful for getting the number
   of items in a list or map, or the number of characters in a string.
*/
MincFloat
_minc_len(const MincValue args[], int nargs)
{
   unsigned long len = 0;

    if (nargs != 1) {
      minc_warn("len: must have one argument");
    }
    else {
      switch (args[0].dataType() ) {
         case MincFloatType:
            len = 1;
          break;
         case MincStringType:
          {
              MincString string = (MincString)args[0];
              len = string ? strlen(string) : 0;
          }
          break;
         case MincHandleType:
            /* NB: To get length of a table, call tablelen(handle) */
            len = 1;
            break;
         case MincListType:
          {
              MincList *list = (MincList *)args[0];
              len = list ? list->len : 0;
          }
          break;
         case MincMapType:
          {
              MincMap *map = (MincMap *)args[0];
              len = map ? map->len() : 0;
          }
          break;
          case MincStructType:
              minc_warn("len: cannot ask for length of a struct");
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
_minc_interp(const MincValue args[], int nargs)
{
	MincFloat outValue = -1;
	if (nargs != 2)
		minc_warn("interp: must have two arguments (list, fraction)");
	else {
		if (args[0].dataType() != MincListType) {
			minc_warn("interp: first argument must be a list");
			return -1.0;
		}
        else if (!(bool)args[0]) {
            minc_warn("interp: list is NULL");
            return -1.0;
        }
        if (args[1].dataType() != MincFloatType) {    // must pass a float as fractional value
            minc_warn("interp: second argument must be a float");
            return -1.0;
        }
		MincValue *data = ((MincList*)args[0])->data;
		int len = ((MincList*)args[0])->len;
		// Deal with degenerate cases
		if (len == 0)
			return 0.0;
		else if (len == 1)
			return (MincFloat)data[0];
		MincFloat fraction = (MincFloat)args[1];
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
/* Given an item (float, string, list, handle, etc.), return the index of the item within
   the given list, or -1 if the item is not in the list.  Example:

      list = {1, 2, "three", 4}
      id = index(list, 2)

   <id> equals 1 after this call.
*/
MincFloat
_minc_index(const MincValue args[], int nargs)
{
   int i, len, index = -1;
   MincDataType argtype;
   MincValue *data;

   if (nargs != 2) {
      minc_warn("index: must have two arguments (list, item_to_find)");
      return -1.0;
   }
   if (!(bool)args[0]) {
      minc_warn("index: container is NULL");
      return -1.0;
   }
   else if (args[0].dataType() != MincListType) {
      minc_warn("index: container must be a list");
      return -1.0;
   }
   argtype = args[1].dataType() ;
   assert(argtype != MincVoidType);
   MincList *list = (MincList *)args[0];
   assert(list != NULL);
   len = list->len;
   data = list->data;

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

/* ---------------------------------------------------------------- contains -- */
/* Given an item (float, string, list, handle, etc.), return 1 if the item is contained
   in the given list or map, or 0 if the item is not found.
 */
MincFloat
_minc_contains(const MincValue args[], int nargs)
{
    if (nargs != 2) {
        minc_warn("contains: must have two arguments (container, item_to_find)");
        return 0;
    }
    if (!(bool)args[0]) {
        minc_warn("contains: container is NULL");
        return -1.0;
    }
    MincDataType argtype = args[1].dataType() ;
    assert(argtype != MincVoidType);

    switch(args[0].dataType()) {
        case MincListType: {
            return _minc_index(args, nargs) != -1.0;
        }
        case MincMapType: {
            MincMap *theMap = (MincMap *) args[0];
            return theMap->contains(args[1]);
        }
        case MincStringType:
            if (args[1].dataType() != MincStringType) {
                minc_warn("contains: search argument must be a string if examining a string");
                return 0;
            }
            else {
                MincString theString = (MincString) args[0];
                MincString theNeedle = (MincString) args[1];
                return (theNeedle != NULL) ? strstr(theString, theNeedle) != NULL ? 1 : 0 : 0;
            }
        default:
            minc_warn("contains: container must be a string, list, or map");
            return 0;
    }
}

MincFloat
_minc_remove(const MincValue args[], int nargs) {
    if (nargs != 2) {
        minc_warn("remove: must have two arguments (container, item_to_remove)");
        return 0;
    }
    else if (!(bool)args[0]) {
        minc_warn("remove: container is NULL");
        return -1.0;
    }
    MincDataType argtype = args[1].dataType();
    assert(argtype != MincVoidType);
    switch (args[0].dataType()) {
        case MincListType: {
            int item_index = (int)_minc_index(args, nargs);
            if (item_index != -1.0) {
                MincList *theList = (MincList *) args[0];
                return theList->removeAtIndex(item_index);
            }
            return 0;
        }
        case MincMapType: {
            MincMap *theMap = (MincMap *) args[0];
            return theMap->remove(args[1]);
        }
        default:
            minc_warn("remove: container must be a list or a map");
            return -1;
    }
}

MincFloat
_minc_insert(const MincValue args[], int nargs) {
    if (nargs != 3) {
        minc_warn("insert: must have three arguments (container, item_to_insert, insert_index)");
        return 0;
    }
    else if (!(bool)args[0]) {
        minc_warn("insert: container is NULL");
        return -1.0;
    }
    int insert_index = (int)(MincFloat)args[2];
    MincDataType insertType = args[1].dataType();
    assert(insertType != MincVoidType);
    switch (args[0].dataType()) {
        case MincListType: {
            if (insert_index >= 0) {
                MincList *theList = (MincList *) args[0];
                return theList->insertAtIndex(args[1], insert_index);
            }
            return -1;
        }
        default:
            minc_warn("remove: container must be a list");
            return -1;
    }
}

/* ------------------------------------------------------------------ type -- */
/* Print the object type of the argument: float, string, handle, list, mincfunction, struct.
*/
MincString
_minc_type(const MincValue args[], int nargs)
{
   if (nargs != 1) {
      minc_warn("type: must have one argument");
      return NULL;
   }
   return _make_type_string(args[0].dataType());
}

/* ------------------------------------------------------------------ tostring -- */
/* Return the passed in (double) argument as a string type.
 */
MincString
_minc_tostring(const MincValue args[], int nargs)
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

/* ------------------------------------------------------------------ substring -- */
/* Return the portion of the string between index0 and index1.
*/
MincString
_minc_substring(const MincValue args[], int nargs)
{
    if (nargs != 3) {
        minc_warn("substring: must have three arguments (string, start_index, end_index)");
        return NULL;
    }
    if (args[0].dataType() != MincStringType) {
        minc_warn("substring: first argument must be a string");
        return 0;
    }
    if (args[1].dataType() != MincFloatType || args[2].dataType() != MincFloatType) {
        minc_warn("substring: second and third arguments must be floats");
        return 0;
    }
    int startIdx = (int)(MincFloat)args[1];
    int endIdx = (int)(MincFloat)args[2];
    MincString theString = (MincString) args[0];
    int len = strlen(theString);
    if (startIdx < 0 || endIdx <= startIdx) {
        minc_warn("substring: illegal indices");
        return 0;
    }
    if (endIdx > len - 1) {
        minc_warn("substring: end index out of range - using string endpoint");
        endIdx = len - 1;
    }
    int newlen = len - startIdx + 1;
    char *sbuffer = new char [newlen];
    strncpy(sbuffer, &theString[startIdx], newlen);
    sbuffer[endIdx - startIdx] = '\0';
    return strdup(sbuffer);
}

// Utilities for calling methods on MincList objects

static float list_append(MincList *inList, const MincValue arglist[])
{
    int newSize = inList->len + 1;
    inList->resize(newSize);
    const MincValue &item = arglist[0];
    inList->data[newSize-1] = item;
    return 1;
}

static MincValue list_min(MincList *inList)
{
    MincValue &val = inList->data[0];
    for (int i = 0; i < inList->len; ++i) {
        MincValue &item = inList->data[i];
        if (item < val) {
            val = item;
        }
    }
    return val;
}

static MincValue list_max(MincList *inList)
{
    MincValue &val = inList->data[0];
    for (int i = 0; i < inList->len; ++i) {
        MincValue &item = inList->data[i];
        if (item > val) {
            val = item;
        }
    }
    return val;
}

int call_list_method(MincValue &object, const char *methodName, const MincValue arglist[], int nargs, MincValue *retval)
{
    MincList *theList = (MincList *)object;
    if (theList == NULL) {
        minc_die("%s: NULL list", methodName);
    }
    int found = 1;
    if (strcmp (methodName, "append") == 0) {
        *retval = list_append(theList, arglist);
    }
    else if (strcmp (methodName, "min") == 0) {
        *retval = list_min(theList);
    }
    else if (strcmp (methodName, "max") == 0) {
        *retval = list_max(theList);
    }
    else {
        found = 0;
    }
    return found;
}


// concatArgs puts the method object on the beginning of the arglist so we can use the
// older builtin functions.

void concatArgs(MincValue newArgs[], MincValue &object, const MincValue arglist[], int nargs)
{
    newArgs[0] = object;
    for (int in = 0, out = 1; in < nargs; ++in, ++out) {
        newArgs[out] = arglist[in];
    }
}

int call_object_method(MincValue &object, const char *methodName, const MincValue arglist[], int nargs, MincValue *retval)
{
    if (strcmp (methodName, "contains") == 0) {
        MincValue args[2];
        concatArgs(args, object, arglist, nargs);
        *retval = (MincFloat) _minc_contains(args, 2);
    }
    else if (strcmp (methodName, "remove") == 0) {
        MincValue args[2];
        concatArgs(args, object, arglist, nargs);
        *retval = (MincFloat) _minc_remove(args, 2);
    }
    else if (strcmp (methodName, "insert") == 0) {
        MincValue args[3];
        concatArgs(args, object, arglist, nargs);
        *retval = (MincFloat) _minc_insert(args, 3);
    }
    else if (strcmp (methodName, "index") == 0) {
        MincValue args[2];
        concatArgs(args, object, arglist, nargs);
        *retval = (MincFloat) _minc_index(args, 2);
    }
    else if (strcmp (methodName, "len") == 0) {
        *retval = (MincFloat) _minc_len((const MincValue *) &object, 1);
    }
    else if (strcmp (methodName, "print") == 0) {
        *retval = (MincFloat) _minc_print((const MincValue *) &object, 1);
    }
    else if (strcmp(methodName, "type") == 0) {
        *retval = (MincString) _minc_type((const MincValue *) &object, 1);
    }
    // Check for list-specific methods
    else if (object.dataType() == MincListType) {
        return call_list_method(object, methodName, arglist, nargs, retval) == 1;
    }
    else {
        return false;
    }
    return true;
}

