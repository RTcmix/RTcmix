#include <rtcmix_types.h>
#include "utils.h"
#include <PField.h>
#include <string.h>		// for strcmp()
#include <stdlib.h>		// for free()
#include <ugens.h>

// NOTE:  For now, the Arg class does not reference and dereference the
// underlying Handle.  This is OK because Args are always temporary storage
// and their scope is always local within the parser.

Arg::~Arg() {
	if (_type == ArrayType) {
		if (_val.array->data)
			free(_val.array->data);
		free(_val.array);
	}
//	else if (_type == HandleType)
//		unrefHandle(_val.handle);
}

bool
Arg::operator == (const char *str) const {
	return isType(StringType) && !strcmp(_val.string, str);
}

Arg::Arg(const Arg &rhs) {
	*this = rhs;
}

Arg & Arg::operator = (const Arg &rhs) {
	assert(_type == VoidType);		// for now, we only allow assigning into empty instances
	_type = rhs._type;
	switch (_type) {
		case DoubleType:
			_val.number = rhs._val.number;
			break;
		case StringType:
			_val.string = rhs._val.string;
			break;
		case HandleType:
			_val.handle = rhs._val.handle;
			break;
		case ArrayType:
			_val.array = (Array *) malloc(sizeof(Array));
			_val.array->data = (double *) malloc(rhs._val.array->len * sizeof(double));
			if (_val.array->data != NULL) {
				memcpy(_val.array->data, rhs._val.array->data, rhs._val.array->len * sizeof(double));
				_val.array->len = rhs._val.array->len;
			}
			break;
		case VoidType:
			break;
	}
	return *this;
}

void
Arg::operator = (const Handle h) {
	_type = HandleType;
	_val.handle = h;
//	refHandle(h);
}

void
Arg::printInline(FILE *stream) const
{
	switch (type()) {
	case DoubleType:
		RTFPrintfCat(stream, "%g ", _val.number);
		break;
	case StringType:
		RTFPrintfCat(stream, "\"%s\" ", _val.string);
		break;
	case HandleType:
		if (_val.handle != NULL) {
			switch (_val.handle->type) {
			case PFieldType:
			{
				// Print PField start and end values.
				PField *pf = (PField *) _val.handle->ptr;
				double start = pf->doubleValue(0);
				double end = pf->doubleValue(1.0);
				RTFPrintfCat(stream, "PF:[%g,...,%g] ", start, end);
				break;
			}
			case InstrumentPtrType:
				RTFPrintfCat(stream, "Inst:%p ", _val.handle->ptr);
				break;
			case AudioStreamType:
				RTFPrintfCat(stream, "AudioStr:%p", _val.handle->ptr);
				break;
			default:
				RTFPrintfCat(stream, "Unknown ");
				break;
			}
		}
		else
			RTFPrintfCat(stream, "NULL ");
		break;
	case ArrayType:
		RTFPrintfCat(stream, "[%g,...,%g] ", _val.array->data[0],
				_val.array->data[_val.array->len - 1]);
		break;
	default:
		break;
	}
}

