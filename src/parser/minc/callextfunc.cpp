/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <RTcmix.h>
#include "minc_internal.h"
#include <handle.h>
#include <rtcmix_types.h>
#include <prototypes.h>
#include <PField.h>

static Arg * minc_list_to_arglist(const char *funcname, const MincListElem *inList, const int inListLen, Arg *inArgs, int *pNumArgs)
{
	int oldNumArgs = *pNumArgs;
	int n = 0, newNumArgs = oldNumArgs + inListLen;
	// Create expanded array
	Arg *newArgs = new Arg[newNumArgs];
	if (newArgs == NULL)
		return NULL;
	if (inArgs != NULL) {
		// Copy existing args to new array
		for (; n < oldNumArgs; ++n) {
			newArgs[n] = inArgs[n];
		}
	}
	for (int i = 0; n < newNumArgs; ++i, ++n) {
		switch (inList[i].type) {
			case MincVoidType:
				minc_die("call_external_function: %s(): invalid argument type", funcname);
				delete [] newArgs;
				return NULL;
			case MincFloatType:
				newArgs[n] = (double) inList[i].val.number;
				break;
			case MincStringType:
				newArgs[n] = inList[i].val.string;
				break;
			case MincHandleType:
				newArgs[n] = (Handle) inList[i].val.handle;
				break;
			case MincListType:
				if (inList[i].val.list == NULL) {
					minc_die("can't pass a null list (arg %d) to RTcmix function %s()", n, funcname);
					return NULL;
				}
				if (inList[i].val.list->len <= 0) {
					minc_die("can't pass an empty list (arg %d) to RTcmix function %s()", n, funcname);
					delete [] newArgs;
					return NULL;
				}
				else {
					minc_die("for now, no nested lists can be passed to RTcmix function %s()", funcname);
					delete [] newArgs;
					return NULL;
				}
		}
	}
	*pNumArgs = newNumArgs;
	return newArgs;
}

int
call_external_function(const char *funcname, const MincListElem arglist[],
	const int nargs, MincListElem *return_value)
{
	int result, numArgs = nargs;
	Arg retval;

	Arg *rtcmixargs = new Arg[nargs];
	if (rtcmixargs == NULL)
		return -1;

	// Convert arglist for passing to RTcmix function.
	for (int i = 0; i < nargs; i++) {
		switch (arglist[i].type) {
		case MincFloatType:
			rtcmixargs[i] = arglist[i].val.number;
			break;
		case MincStringType:
			rtcmixargs[i] = arglist[i].val.string;
			break;
		case MincHandleType:
			rtcmixargs[i] = (Handle) arglist[i].val.handle;
			break;
		case MincListType:
			if (arglist[i].val.list == NULL) {
				minc_die("can't pass a null list (arg %d) to RTcmix function %s()", i, funcname);
				return -1;
			}
			if (arglist[i].val.list->len <= 0) {
				minc_die("can't pass an empty list (arg %d) to RTcmix function %s()", i, funcname);
				return -1;
			}
			// If list is final argument to function, treat its contents as additional function arguments
			if (i == nargs-1) {
				int argCount = i;
				Arg *newargs = minc_list_to_arglist(funcname, arglist[i].val.list->data, arglist[i].val.list->len, rtcmixargs, &argCount);
				delete [] rtcmixargs;
				if (newargs == NULL)
					return -1;
				rtcmixargs = newargs;
				numArgs = argCount;
			}
			// If list contains only floats, convert and pass it along.
			else {
				Array *newarray = (Array *) emalloc(sizeof(Array));
				if (newarray == NULL)
					return -1;
				assert(sizeof(*newarray->data) == sizeof(double));	// because we cast MincFloat to double here
				newarray->data = (double *) float_list_to_array(arglist[i].val.list);
				if (newarray->data != NULL) {
					newarray->len = arglist[i].val.list->len;
					rtcmixargs[i] = newarray;
				}
				else {
					minc_die("can't pass a mixed-type list (arg %d) to RTcmix function %s()", i, funcname);
					free(newarray);
					return -1;
				}
			}
			break;
		default:
			minc_die("call_external_function: %s(): arg %d: invalid argument type",
					 funcname, i);
			return -1;
			break;
		}
	}

	result = RTcmix::dispatch(funcname, rtcmixargs, numArgs, &retval);
   
	// Convert return value from RTcmix function.
	switch (retval.type()) {
	case DoubleType:
		return_value->type = MincFloatType;
		return_value->val.number = (MincFloat) retval;
		break;
	case StringType:
		return_value->type = MincStringType;
		return_value->val.string = (MincString) retval;
		break;
	case HandleType:
		return_value->type = MincHandleType;
		return_value->val.handle = (MincHandle) (Handle) retval;
		if (return_value->val.handle) {
			ref_handle(return_value->val.handle);
		}
		break;
	case ArrayType:
#ifdef NOMORE
// don't think functions will return non-opaque arrays to Minc, but if they do,
// these should be converted to MincListType
		return_value->type = MincArrayType;
		{
			Array *array = (Array *) retval;
			return_value->val.array.len = array->len;
			return_value->val.array.data = array->data;
		}
#endif
		break;
	default:
		break;
	}

	delete [] rtcmixargs;

	return result;
}

static Handle _createPFieldHandle(PField *pfield)
{
	Handle handle = (Handle) malloc(sizeof(struct _handle));
	handle->type = PFieldType;
	handle->ptr = (void *) pfield;
	pfield->ref();
	handle->refcount = 0;
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

MincHandle minc_binop_handle_float(const MincHandle mhandle,
	const MincFloat val, OpKind op)
{
	DPRINT("minc_binop_handle_float (handle=%p, val=%f\n", mhandle, val);

	// Extract PField from MincHandle.
	Handle handle = (Handle) mhandle;
	assert(handle->type == PFieldType);
	PField *pfield1 = (PField *) handle->ptr;

	// Create ConstPField for MincFloat.
	PField *pfield2 = new ConstPField(val);

	// Create PField using appropriate operator.
	PField *outpfield = createBinopPField(pfield1, pfield2, op);

	return (MincHandle) _createPFieldHandle(outpfield);
}

MincHandle minc_binop_float_handle(const MincFloat val,
	const MincHandle mhandle, OpKind op)
{
	DPRINT("minc_binop_float_handle (val=%f, handle=%p\n", val, mhandle);

	// Create ConstPField for MincFloat.
	PField *pfield1 = new ConstPField(val);

	// Extract PField from MincHandle.
	Handle handle = (Handle) mhandle;
	assert(handle->type == PFieldType);
	PField *pfield2 = (PField *) handle->ptr;

	// Create PField using appropriate operator.
	PField *outpfield = createBinopPField(pfield1, pfield2, op);

	return (MincHandle) _createPFieldHandle(outpfield);
}

MincHandle minc_binop_handles(const MincHandle mhandle1,
	const MincHandle mhandle2, OpKind op)
{
	DPRINT("minc_binop_handles (handle1=%p, handle2=%p\n", mhandle1, mhandle2);

	// Extract PFields from MincHandles

	Handle handle1 = (Handle) mhandle1;
	Handle handle2 = (Handle) mhandle2;
	assert(handle1->type == PFieldType);
	assert(handle2->type == PFieldType);
	PField *pfield1 = (PField *) handle1->ptr;
	PField *pfield2 = (PField *) handle2->ptr;
	PField *opfield = createBinopPField(pfield1, pfield2, op);

	// create Handle for new PField, return it cast to MincHandle

	return (MincHandle) _createPFieldHandle(opfield);
}

