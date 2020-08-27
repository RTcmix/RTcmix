/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "minc_internal.h"
#include "MincValue.h"
#include <math.h>
#include <stdlib.h>

/* Minc utilities.  By John Gibson, 1/24/2004 */


/* --------------------------------------------------------- is_float_list -- */
/* Return 1 if <list> contains only elements of type MincFloat; otherwise,
   return 0.
*/
int
is_float_list(const MincList *list)
{
   int i;

   for (i = 0; i < list->len; i++)
      if (list->data[i].dataType() != MincFloatType)
         return 0;

   return 1;
}


/* --------------------------------------------------- float_list_to_array -- */
/* Given a MincList containing only floats, convert the data into a new array
   of MincFloat, and return this.  If the MincList contains any non-float data,
   return NULL.  If there isn't enough memory for a new array, the program will
   die before this function returns.
*/
MincFloat *
float_list_to_array(const MincList *list)
{
   int i;
   MincFloat *array = NULL;
   if (list->len > 0)
	  array = (MincFloat *) emalloc(list->len * sizeof(MincFloat));
   if (array == NULL)
      return NULL;
   for (i = 0; i < list->len; i++) {
      if (list->data[i].dataType() != MincFloatType) {
         free(array);
         return NULL;
      }
      array[i] = (MincFloat)list->data[i];
   }

   return array;
}


/* --------------------------------------------------- array_to_float_list -- */
/* Convert the given array of MincFloat into a new MincList, and return this.
   If there isn't enough memory for a new MincList, the program will die
   before this function returns.
*/
MincList *
array_to_float_list(const MincFloat *array, const int len)
{
   int i;
   MincList *list = new MincList(len);

   for (i = 0; i < len; i++) {
      list->data[i] = array[i];
   }

   return list;
}

const char *MincTypeName(MincDataType type)
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
        case MincMapType:
            return "map";
        case MincStructType:
            return "struct";
	}
	return NULL;
}

static int score_line_offset = 0;
void increment_score_line_offset(int offset)
{
	score_line_offset = offset;
}
int get_score_line_offset()
{
	return score_line_offset;
}

/* Has error-checking for malloc built in. */
char *
emalloc(long nbytes)
{
    char *s;
    
    s = (char *) malloc(nbytes);
    if (s == NULL)
        sys_error("system out of memory");
    
#ifndef NO_EMALLOC_DEBUG
    DPRINT("emalloc: nbytes=%d, ptr=%p\n", nbytes, s);
#endif
    return s;
}

void efree(void *mem)
{
#ifndef NO_EMALLOC_DEBUG
    DPRINT("efree: ptr=%p\n", mem);
#endif
    free(mem);
}

/* Returns an index to a hash bucket. */
int
hash(const char *s)
{
    int i = 0;
    
    while (*s) {
        i = (((unsigned int) *s + i) % HASHSIZE);
        s++;
    }
    return i;
}

/* floating point comparisons:
 f1 < f2   ==> -1
 f1 == f2  ==> 0
 f1 > f2   ==> 1
 */
int
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


