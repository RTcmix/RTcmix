#include <rtcmix_types.h>
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
		fprintf(stream, "Handle:%p ", val.handle);
		break;
	case ArrayType:
		fprintf(stream, "[%g...%g] ", val.array->data[0],
				val.array->data[val.array->len - 1]);
		break;
	default:
		break;
	}
}

