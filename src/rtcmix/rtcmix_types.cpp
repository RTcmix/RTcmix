#include <rtcmix_types.h>
#include <PField.h>
#include <stdlib.h>		// for free()

Arg::~Arg() { if (type == ArrayType) free(val.array); }

void 
Arg::printInline(FILE *stream) const
{
	switch (type) {
	case DoubleType:
		fprintf(stream, "%g ", val.number);
		break;
	case StringType:
		fprintf(stream, "\"%s\" ", val.string);
		break;
	case HandleType:
		fprintf(stream, "%sHandle:%p",
				val.handle->type == PFieldType ? "PF" :
				val.handle->type == InstrumentPtrType ? "Inst" :
				val.handle->type == PFieldType ? "AudioStr" : "Unknown",
				val.handle);
		if (val.handle->type == PFieldType) {
			// Print PField start and end values.
			PField *pf = (PField *) val.handle->ptr;
			double start = pf->doubleValue(0);
			double end = pf->doubleValue(1);
			fprintf(stream, ":[%g...%g] ", start, end);
		}
		else
			fprintf(stream, " ");
		break;
	case ArrayType:
		fprintf(stream, "[%g...%g] ", val.array->data[0],
				val.array->data[val.array->len - 1]);
		break;
	default:
		break;
	}
}

