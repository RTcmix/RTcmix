#include <rtcmix_types.h>
#include <PField.h>
#include <string.h>		// for strcmp()
#include <stdlib.h>		// for free()

Arg::~Arg() { if (_type == ArrayType) free(_val.array); }

bool
Arg::operator == (const char *str) const {
	return isType(StringType) && !strcmp(_val.string, str);
}

void 
Arg::printInline(FILE *stream) const
{
	switch (type()) {
	case DoubleType:
		fprintf(stream, "%g ", _val.number);
		break;
	case StringType:
		fprintf(stream, "\"%s\" ", _val.string);
		break;
	case HandleType:
		fprintf(stream, "%sHndl:",
				_val.handle->type == PFieldType ? "PF" :
				_val.handle->type == InstrumentPtrType ? "Inst" :
				_val.handle->type == PFieldType ? "AudioStr" : "Unknown");
		if (_val.handle->type == PFieldType) {
			// Print PField start and end values.
			PField *pf = (PField *) _val.handle->ptr;
			double start = pf->doubleValue(0);
			double end = pf->doubleValue(1.0);
			fprintf(stream, ":[%g,...,%g] ", start, end);
		}
		else
			fprintf(stream, " ");
		break;
	case ArrayType:
		fprintf(stream, "[%g,...,%g] ", _val.array->data[0],
				_val.array->data[_val.array->len - 1]);
		break;
	default:
		break;
	}
}

