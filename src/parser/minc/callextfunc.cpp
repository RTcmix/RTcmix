/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "minc_internal.h"
#include <rtcmix_types.h>
#include <parse_dispatch.h>
#include <PField.h>


int
call_external_function(const char *funcname, const MincListElem arglist[],
   const int nargs, MincListElem *return_value)
{
   int i, result, rtcmixargs_array_allocated = 0;
   Arg retval, *rtcmixargs;

   rtcmixargs = (Arg *) emalloc(nargs * sizeof(Arg));
   if (rtcmixargs == NULL)
      return -1;

   /* Convert arglist for passing to RTcmix function. */
   for (i = 0; i < nargs; i++) {
      switch (arglist[i].type) {
         case MincFloatType:
            rtcmixargs[i].type = DoubleType;
            rtcmixargs[i].val.number = arglist[i].val.number;
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
            rtcmixargs[i].val.array = (Array *) emalloc(sizeof(Array));
            if (rtcmixargs[i].val.array == NULL)
               return -1;
            rtcmixargs[i].val.array->data
                        = (double *) float_list_to_array(&arglist[i].val.list);
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
      case DoubleType:
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

static Handle
createPFieldHandle(PField *pfield)
{
   Handle handle = (Handle) malloc(sizeof(struct _handle));
   handle->type = PFieldType;
   handle->ptr = (void *) pfield;
   return handle;
}


static double plus_binop(double x, double y)
{
	return x + y;
}
static double minus_binop(double x, double y)
{
	return x - y;
}
static double mult_binop(double x, double y)
{
	return x * y;
}
static double divide_binop(double x, double y)
{
	return (y != 0.0) ? x / y : 999999999999999999.9;
}
static double mod_binop(double x, double y)
{
	return (int) x % (int) y;
}
static double pow_binop(double x, double y)
{
	return pow(x, y);
}

PField *createBinopPField(PField *pfield1, PField *pfield2, OpKind op)
{
	PField *opfield = NULL;
	PFieldBinaryOperator::Operator binop = NULL;

	// Create appropriate binary operator PField
	
	switch (op) {
	case OpPlus:
		binop = plus_binop;
		break;
	case OpMinus:
		binop = minus_binop;
		break;
	case OpMul:
		binop = mult_binop;
		break;
	case OpDiv:
		binop = divide_binop;
		break;
	case OpMod:
		binop = mod_binop;
		break;
	case OpPow:
		binop = pow_binop;
		break;
	case OpNeg:
	default:
		minc_internal_error("invalid binary handle operator");
		return NULL;
	}

	// create new Binop PField, return it cast to MincHandle

	return new PFieldBinaryOperator(pfield1, pfield2, binop);
}

MincHandle
minc_binop_handle_float(const MincHandle mhandle, const MincFloat val, OpKind op)
{
   Handle return_handle;

   DPRINT2("minc_binop_handle_float (handle=%p, val=%f\n", handle, val);

	// Extract PField from MincHandle.
	Handle handle = (Handle) mhandle;
	assert(handle->type == PFieldType);
	PField *pfield1 = (PField *) handle->ptr;

	// Create ConstPField for MincFloat.
	PField *pfield2 = new ConstPField(val);
	
    // Create PField using appropriate operator.
    PField *outpfield = createBinopPField(pfield1, pfield2, op);
	
	return (MincHandle) createPFieldHandle(outpfield);
}

MincHandle
minc_binop_handles(const MincHandle mhandle1, const MincHandle mhandle2, OpKind op)
{
	DPRINT2("minc_binop_handles (handle1=%p, handle2=%p\n", handle1, handle2);

	// Extract PFields from MincHandles
	
	Handle handle1 = (Handle) mhandle1;
	Handle handle2 = (Handle) mhandle2;
	assert(handle1->type == PFieldType);
	assert(handle2->type == PFieldType);
	PField *pfield1 = (PField *) handle1->ptr;
	PField *pfield2 = (PField *) handle2->ptr;
	PField *opfield = createBinopPField(pfield1, pfield2, op);
	
	// create Handle for new PField, return it cast to MincHandle

	return (MincHandle) createPFieldHandle(opfield);
}

