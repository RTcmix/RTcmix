/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ugens.h>      // for die, warn
#include "rtcmix_types.h"
#include <mixerr.h>
#include "prototypes.h"
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

struct FunctionEntry {
	FunctionEntry(const char *fname, const char *dsoPath);
	~FunctionEntry();
	char *funcName;
	char *dsoPath;
	struct FunctionEntry *next;
};

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

    bool autoload = Option::autoLoad();
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
         if (!autoload)
             rtcmix_warn("addfunc", "Function '%s' already introduced",
                  this_node->func_label);
          delete this_node;
         return;
      }
#endif
   }
//   RTPrintf("addfunc: Function '%s' introduced at %p\n", this_node->func_label, this_node->func_ptr.legacy_return);
   cur_node->next = this_node;
} 


/* ----------------------------------------------------------- freefuncs -- */
void
RTcmix::freefuncs()
{
	for (RTcmixFunction *cur_node = _func_list; cur_node; ) {
		RTcmixFunction *next = cur_node->next;
		delete cur_node;
		cur_node = next;
	}
	_func_list = NULL;
	
	// DAS added 01/2014
	
	for (FunctionEntry *entry = _functionRegistry; entry; ) {
		FunctionEntry *next = entry->next;
		delete entry;
		entry = next;
	}
	_functionRegistry = NULL;
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

   if (Option::print() >= MMP_PRINTALL) {
      RTPrintf("============================\n");
      RTPrintfCat("%s:  ", funcname);
      for (i = 0; i < nargs; i++) {
         arglist[i].printInline(stdout);
      }
      RTPrintf("\n");
   }
}

/* ------------------------------------------------------------- checkfunc -- */
int
RTcmix::checkfunc(const char *funcname, const Arg arglist[], const int nargs,
		          Arg *retval)
{
   RTcmixFunction *func;

   func = ::findfunc(_func_list, funcname);

   // If we did not find it, try loading it from our list of registered DSOs.
   if (func == NULL) {
      if (findAndLoadFunction(funcname) == 0) {
         func = ::findfunc(_func_list, funcname);
		  if (func == NULL) {
			   mixerr = MX_FNAME;
               return -1;
		  }
      }
      else {
		  mixerr = MX_FNAME;
		  return -1;
	  }
   }

   /* function found, so call it */

   ::_printargs(funcname, arglist, nargs);

   int status = 0;

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
               pp[i] = STRING_TO_DOUBLE(theArg);
			   break;
            default:
               return die(NULL, "%s: arguments must be numbers or strings.", funcname);
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
	  {
      Handle retHandle = (Handle) (*(func->func_ptr.handle_return))
                                                      (arglist, nargs);
	  if (retHandle == NULL) {
		  status = -1;
		  mixerr = MX_FAIL;
	  }
	  *retval = retHandle;
	  }
      break;
   case StringType:
	  {
      const char *retString = (const char *) (*(func->func_ptr.string_return))
                                                      (arglist, nargs);
	  if (retString == NULL) {
		  status = -1;
		  mixerr = MX_FAIL;
	  }
	  *retval = retString;
	  }
      break;
   default:
	  die(NULL, "%s: unhandled return type: %d", funcname, (int)func->return_type);
	  status = -1;
	  mixerr = MX_FAIL;
      break;
   }

   return status;
}

// Code for function/DSO registry, which allows RTcmix to auto-load a DSO
// for a given function.

FunctionEntry::FunctionEntry(const char *fname, const char *dso_path)
	: funcName(strdup(fname)), dsoPath(strdup(dso_path)), next(NULL)
{
}

FunctionEntry::~FunctionEntry()
{
	free(dsoPath);
	free(funcName);
}

static FunctionEntry *
findFunctionEntry(FunctionEntry *entry, const char *funcname)
{
	while (entry != NULL) {
		if (!strcmp(entry->funcName, funcname))
			return entry;
		entry = entry->next;
	}
	return NULL;
}

static const char *
getDSOPath(FunctionEntry *entry, const char *funcname)
{
	FunctionEntry * fentry = findFunctionEntry(entry, funcname);
	if (fentry != NULL)
        return fentry->dsoPath;
	return NULL;
}

extern "C" double m_load(float *, int, double *);	// loader.c

/* --------------------------------------------------- findAndLoadFunction -- */

// Called by RTcmix::checkfunc() to allow auto-loading of DSOs.

int
RTcmix::findAndLoadFunction(const char *funcname)
{
	const char *path;
	int status = -1;
	if ((path = ::getDSOPath(_functionRegistry, funcname)) != NULL) {
		char fullDSOPath[128];
		float p[1];
		double pp[1];
		sprintf(fullDSOPath, "%s.so", path);
		p[0] = 0;
		pp[0] = STRING_TO_DOUBLE(fullDSOPath);
//        RTPrintf("findAndLoadFunction: calling load() on '%s' for function '%s'\n", fullDSOPath, funcname);
		if (m_load(p, 1, pp) == 1)
			status = 0;
		else
			status = -1; 
	}
	return status;
}

/* ------------------------------------------------------ registerFunction -- */

// This is called by each DSO's registerMe() function to register a given
// Minc command name (function name) with a particular DSO name.  When
// RTcmix::checkfunc() fails to find a functions, it calls findAndLoadFunction()
// to search the function/DSO database for a matching DSO.

int 
RTcmix::registerFunction(const char *funcName, const char *dsoPath)
{
	const char *path;
	if ((path = ::getDSOPath(_functionRegistry, funcName)) == NULL) {
		FunctionEntry *newEntry = new FunctionEntry(funcName, dsoPath);
		newEntry->next = _functionRegistry;
		_functionRegistry = newEntry;
		RTPrintf("RTcmix::registerFunction: registered function '%s' for dso '%s'\n",
				funcName, dsoPath);
		return 0;
	}
	else {
		rtcmix_warn("RTcmix::registerFunction",
			  "'%s' already registered for DSO '%s'", funcName, path);
		return -1;
	}
}

#include <dirent.h>
#include "DynamicLib.h"

/* ------------------------------------------------------ registerDSOs -- */
// This is called at initialization time to scan a supplied semicolon-separated
// list of directories for DSOs (files which begin with "lib").  When found,
// each is searched for a "registerSelf()" function, which is called.

typedef int (*RegisterFunction)();

int
RTcmix::registerDSOs(const char *pathList)
{
	const char *list = pathList;
	while (list != NULL) {
		char path[1024];
		long itemLen;
		const char *nextItem = strchr(list, ':');
		if (nextItem != NULL) {
			itemLen = nextItem - list;
			++nextItem;		// skip semicolon
		}
		else {
			itemLen = strlen(list);
		}
		strncpy(path, list, itemLen);
		path[itemLen] = '\0';
		
		DIR *dsoDir = opendir(path);
		if (dsoDir != NULL) {
			struct dirent *entry;
			while ((entry = readdir(dsoDir)) != NULL) {
				if (strncmp(entry->d_name, "lib", 3) == 0) {
					char fullPath[1024];
					sprintf(fullPath, "%s/%s", path, entry->d_name);
					DynamicLib dso;
					if (dso.load(fullPath) == 0) {
						RegisterFunction registerMe = NULL;
//						RTPrintf("opened DSO '%s'\n", fullPath);
						if (dso.loadFunction(&registerMe, "registerSelf") == 0)
						{
//							RTPrintf("\tcalling register function.\n");
							(*registerMe)();
						}
						dso.unload();
					}
				}
			}
			closedir(dsoDir);
		}
		list = nextItem;
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

int
registerFunction(const char *funcName, const char *dsoPath)
{
	return RTcmix::registerFunction(funcName, dsoPath);
}
