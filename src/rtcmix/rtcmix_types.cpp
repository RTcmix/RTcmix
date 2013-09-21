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
		RTFPrintf(stream, "%g ", _val.number);
		break;
	case StringType:
		RTFPrintf(stream, "\"%s\" ", _val.string);
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
				RTFPrintf(stream, "PF:[%g,...,%g] ", start, end);
				break;
			}
			case InstrumentPtrType:
				RTFPrintf(stream, "Inst:%p ", _val.handle->ptr);
				break;
			case AudioStreamType:
				RTFPrintf(stream, "AudioStr:%p", _val.handle->ptr);
				break;
			default:
				RTFPrintf(stream, "Unknown ");
				break;
			}
		}
		else
			RTFPrintf(stream, "NULL ");
		break;
	case ArrayType:
		RTFPrintf(stream, "[%g,...,%g] ", _val.array->data[0],
				_val.array->data[_val.array->len - 1]);
		break;
	default:
		break;
	}
}

