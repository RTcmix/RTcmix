/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <RTcmix.h>
#include "minc_internal.h"
#include "MincValue.h"
#include <handle.h>
#include <rtcmix_types.h>
#include <prototypes.h>
#include <PField.h>

static Arg * minc_list_to_arglist(const char *funcname, const MincValue *inList, const int inListLen, Arg *inArgs, int *pNumArgs)
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
        minc_try {
		switch (inList[i].dataType()) {
            default:
			case MincVoidType:
				minc_die("call_external_function: %s(): invalid argument type", funcname);
				return NULL;
			case MincFloatType:
				newArgs[n] = (MincFloat) inList[i];
				break;
			case MincStringType:
				newArgs[n] = (MincString)inList[i];
				break;
			case MincHandleType:
				newArgs[n] = (Handle) (MincHandle)inList[i];
				break;
			case MincListType:
				if ((MincList *)inList[i] == NULL) {
					minc_die("can't pass a null list (arg %d) to RTcmix function %s()", n, funcname);
					return NULL;
				}
				if (((MincList *)inList[i])->len <= 0) {
					minc_die("can't pass an empty list (arg %d) to RTcmix function %s()", n, funcname);
					return NULL;
				}
				else {
					minc_die("for now, no nested lists can be passed to RTcmix function %s()", funcname);
					return NULL;
				}
                break;
            case MincMapType:
                minc_die("for now, maps cannot be passed to RTcmix function %s()", funcname);
                return NULL;
            case MincStructType:
                minc_die("for now, structs cannot be passed to RTcmix function %s()", funcname);
                return NULL;
            case MincFunctionType:
                minc_die("for now, functions cannot be passed to RTcmix function %s()", funcname);
                return NULL;
		}
        } minc_catch(delete [] newArgs;)
	}
	*pNumArgs = newNumArgs;
	return newArgs;
}

int
call_external_function(const char *funcname, const MincValue arglist[],
	const int nargs, MincValue *return_value)
{
	int result, numArgs = nargs;
	Arg retval;

	Arg *rtcmixargs = new Arg[nargs];
	if (rtcmixargs == NULL)
		return MEMORY_ERROR;

    minc_try {
	// Convert arglist for passing to RTcmix function.
	for (int i = 0; i < nargs; i++) {
		switch (arglist[i].dataType()) {
		case MincFloatType:
			rtcmixargs[i] = (MincFloat)arglist[i];
			break;
		case MincStringType:
			rtcmixargs[i] = (MincString)arglist[i];
			break;
		case MincHandleType:
			rtcmixargs[i] = (Handle) (MincHandle)arglist[i];
#ifdef EMBEDDED
			if ((Handle)rtcmixargs[i] == NULL) {
				minc_die("can't pass a null handle (arg %d) to RTcmix function %s()", i, funcname);
				return PARAM_ERROR;
			}
#endif
			break;
		case MincListType:
			{
			MincList *list = (MincList *)arglist[i];
			if (list == NULL) {
				minc_die("can't pass a null list (arg %d) to RTcmix function %s()", i, funcname);
				return PARAM_ERROR;
			}
			if (list->len <= 0) {
				minc_die("can't pass an empty list (arg %d) to RTcmix function %s()", i, funcname);
				return PARAM_ERROR;
			}
			// If list is final argument to function, treat its contents as additional function arguments
			if (i == nargs-1) {
				int argCount = i;
				Arg *newargs = minc_list_to_arglist(funcname, list->data, list->len, rtcmixargs, &argCount);
				delete [] rtcmixargs;
				if (newargs == NULL)
					return PARAM_ERROR;
				rtcmixargs = newargs;
				numArgs = argCount;
			}
			// If list contains only floats, convert and pass it along.
			else {
				Array *newarray = (Array *) emalloc(sizeof(Array));
				if (newarray == NULL)
					return MEMORY_ERROR;
				assert(sizeof(*newarray->data) == sizeof(double));	// because we cast MincFloat to double here
				newarray->data = (double *) float_list_to_array(list);
				if (newarray->data != NULL) {
					newarray->len = list->len;
					rtcmixargs[i] = newarray;
				}
				else {
					minc_die("can't pass a mixed-type list (arg %d) to RTcmix function %s()", i, funcname);
					free(newarray);
					return PARAM_ERROR;
				}
			}
			}
			break;
       case MincMapType:
            minc_die("%s(): arg %d: maps not supported as function arguments", funcname, i);
            return PARAM_ERROR;
            break;
       case MincStructType:
            minc_die("%s(): arg %d: structs not supported as function arguments", funcname, i);
            return PARAM_ERROR;
            break;
       case MincFunctionType:
            minc_die("%s(): arg %d: functions not supported as function arguments", funcname, i);
            return PARAM_ERROR;
            break;
		default:
			minc_die("%s(): arg %d: invalid argument type", funcname, i);
			return PARAM_ERROR;
			break;
		}
	}
    } minc_catch(delete [] rtcmixargs;)

	result = RTcmix::dispatch(funcname, rtcmixargs, numArgs, &retval);
   
	// Convert return value from RTcmix function.
	switch (retval.type()) {
	case DoubleType:
		*return_value = (MincFloat) retval;
		break;
	case StringType:
		*return_value = (MincString) retval;
		break;
	case HandleType:
		*return_value = (MincHandle) (Handle) retval;
		break;
	case ArrayType:
       {
//          Functions can return non-opaque arrays to Minc.
//          These are converted to MincListType so they can be accessed as 'list' objects
           Array *array = (Array *) retval;
           MincList *outlist = new MincList(array->len);
           MincValue *dest = outlist->data;
           for (int n=0; n < array->len; ++n) {
               dest[n] = array->data[n];
           }
           *return_value = outlist;
		}
		break;
	default:
		break;
	}

	delete [] rtcmixargs;

	return result;
}

void printargs(const char *funcname, const Arg arglist[], const int nargs)
{
	RTcmix::printargs(funcname, arglist, nargs);
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

    if (mhandle == NULL) {
        minc_warn("Null handle in binary operation");
        return (MincHandle)0;
    }
	// Extract PField from MincHandle.
	Handle handle = (Handle) mhandle;
    if (handle->type != PFieldType) {
        minc_die("Illegal handle type for this operation");
        return (MincHandle)0;
    }
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

    if (mhandle == NULL) {
        minc_warn("Null handle in binary operation");
        return (MincHandle)0;
    }

	// Extract PField from MincHandle.
	Handle handle = (Handle) mhandle;
    if (handle->type != PFieldType) {
        minc_die("Illegal handle type for this operation");
        return (MincHandle)0;
    }
    
    // Create ConstPField for MincFloat.
    PField *pfield1 = new ConstPField(val);
    
	PField *pfield2 = (PField *) handle->ptr;

	// Create PField using appropriate operator.
	PField *outpfield = createBinopPField(pfield1, pfield2, op);

	return (MincHandle) _createPFieldHandle(outpfield);
}

MincHandle minc_binop_handles(const MincHandle mhandle1,
	const MincHandle mhandle2, OpKind op)
{
	DPRINT("minc_binop_handles (handle1=%p, handle2=%p\n", mhandle1, mhandle2);

    if (mhandle1 == NULL || mhandle2 == NULL) {
        minc_warn("Null handle(s) in binary operation");
        return (MincHandle)0;
    }
	// Extract PFields from MincHandles

	Handle handle1 = (Handle) mhandle1;
	Handle handle2 = (Handle) mhandle2;
    if (handle1->type != PFieldType || handle2->type != PFieldType) {
        minc_die("Illegal handle type(s) for this operation");
        return (MincHandle)0;
    }
	PField *pfield1 = (PField *) handle1->ptr;
	PField *pfield2 = (PField *) handle2->ptr;
	PField *opfield = createBinopPField(pfield1, pfield2, op);

	// create Handle for new PField, return it cast to MincHandle

	return (MincHandle) _createPFieldHandle(opfield);
}

