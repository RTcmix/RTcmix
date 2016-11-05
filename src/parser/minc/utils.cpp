/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include "minc_internal.h"

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
      array[i] = (MincFloat)list->data[i].value();
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
   MincList *list;

   list = (MincList *) emalloc(sizeof(MincList));
   if (list == NULL)
      return NULL;
   list->data = (MincListElem *) emalloc(len * sizeof(MincListElem));
   if (list->data == NULL) {
      free(list);
      return NULL;
   }

   for (i = 0; i < len; i++) {
      list->data[i].value() = array[i];
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
	}
	return NULL;
}

#ifdef EMBEDDED
typedef size_t yy_size_t;	// from lex.yy.c
static const char *sGlobalBuffer;
static int sGlobalBufferLength;
static int sBufferOffset;

extern "C" {
void setGlobalBuffer(const char *inBuf, int inBufSize)
{
	sGlobalBuffer = inBuf;
	sGlobalBufferLength = inBufSize;
	sBufferOffset = 0;	// reset
}

int readFromGlobalBuffer(char *buf, yy_size_t *pBytes, int maxbytes)
{
	{
		int  n;
		for (n = 0; n < maxbytes && sBufferOffset < (sGlobalBufferLength-1); ++n)
		{
			buf[n] = sGlobalBuffer[sBufferOffset++];
		}
		*pBytes = n;
	}
	return 0;
}
}

#endif

