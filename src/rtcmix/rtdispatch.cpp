#include "../sys/mixerr.h"

extern "C" double checkInsts(char*, double*, short);

extern "C" {
	double rtdispatch(char *fname, double *pp, short n_args)
	{
		double rv;

		rv = checkInsts(fname, pp, n_args);
		return rv;
	}
}


