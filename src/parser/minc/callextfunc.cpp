/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <assert.h>
#include "minc_internal.h"
#include <rtcmix_types.h>
#include <parse_dispatch.h>


int
call_external_function(const char *funcname, const MincListElem arglist[],
   const int nargs, MincListElem *return_value)
{
   int i, result, rtcmixargs_array_allocated = 0;
   Arg retval, *rtcmixargs;

   rtcmixargs = (Arg *) emalloc(nargs * sizeof(Arg));
   /* NB: emalloc dies if no more memory */

   /* Convert arglist for passing to RTcmix function. */
   for (i = 0; i < nargs; i++) {
      switch (arglist[i].type) {
         case MincFloatType:
            rtcmixargs[i].type = FloatType;
            rtcmixargs[i].val.number = (Float) arglist[i].val.number;
            break;
         case MincStringType:
            rtcmixargs[i].type = StringType;
            rtcmixargs[i].val.string = arglist[i].val.string;
            break;
         case MincHandleType:
            rtcmixargs[i].type = HandleType;
            rtcmixargs[i].val.handle = (Handle) arglist[i].val.handle;
            break;
         case MincListType:
            /* If list contains only floats, convert and pass it along.
               Otherwise, it's an error.
            */
//FIXME: this only works if Float and MincFloat are same size!
            rtcmixargs[i].val.array = (Array *) emalloc(sizeof(Array));
            rtcmixargs[i].val.array->data
                        = (Float *) float_list_to_array(&arglist[i].val.list);
            if (rtcmixargs[i].val.array->data != NULL) {
               rtcmixargs[i].type = ArrayType;
               rtcmixargs[i].val.array->len = arglist[i].val.list.len;
               rtcmixargs_array_allocated = 1;
            }
            else {
               minc_die("can't pass a mixed-type list to an RTcmix function");
               return -1;
            }
            break;
         default:
            minc_die("call_external_function: invalid argument type");
            break;
      }
   }

   result = parse_dispatch(funcname, rtcmixargs, nargs, &retval);

   /* Convert return value from RTcmix function. */
   switch (retval.type) {
      case FloatType:
         return_value->type = MincFloatType;
         return_value->val.number = (MincFloat) retval.val.number;
         break;
      case StringType:
         return_value->type = MincStringType;
         return_value->val.string = retval.val.string;
         break;
      case HandleType:
         return_value->type = MincHandleType;
         return_value->val.handle = (MincHandle) retval.val.handle;
         break;
      case ArrayType:
#ifdef NOMORE
// don't think functions will return non-opaque arrays to Minc, but if they do,
// these should be converted to MincListType
         return_value->type = MincArrayType;
         return_value->val.array.len = retval.val.array->len;
//FIXME: this only works if Float and MincFloat are same size!
         return_value->val.array.data = retval.val.array->data;
         free(retval.val.array);
#endif
         break;
      default:
         break;
   }

   /* Free memory allocated in this function that will no longer be used. */
   if (rtcmixargs_array_allocated) {
      for (i = 0; i < nargs; i++) {
         if (rtcmixargs[i].type == ArrayType)
            free(rtcmixargs[i].val.array);
      }
   }
   free(rtcmixargs);

   return 0;
}

