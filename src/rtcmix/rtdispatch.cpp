/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <prototypes.h>
#include "../sys/mixerr.h"


extern "C" {
	double rtdispatch(char *fname, double *pp, int n_args)
	{
		double rv;

		// BGG -- if an RT instrument was found, "rv" is now a double
	        // coerced from an int that represents an Instrument* pointer
		// to the scheduled object
		rv = checkInsts(fname, pp, n_args);
		return rv;
	}
}


