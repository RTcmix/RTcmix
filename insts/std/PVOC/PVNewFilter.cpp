//
//  PVNewFilter.cpp
//  RTcmixTest
//
//  Created by Douglas Scott on 3/15/14.
//
//

#include "PVNewFilter.h"

PVFilter *
PVNewFilter::create()
{
	return new PVNewFilter;
}

PVNewFilter::PVNewFilter() : gains(0), numgains(0)
{
}

PVNewFilter::~PVNewFilter()
{
	delete [] gains;
}

int
PVNewFilter::run(float *pvdata, int nvals)
{
	float incr = (float)(numgains-1) / nvals;
	float findex = 0.0;
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		int idx = (int) findex;
		float gain = gains[idx];
		float nextgain = gains[idx+1];
		float frac = findex - idx;
		pvdata[ampIdx] *= (gain + frac * (nextgain - gain));
		findex += incr;
	}
	return 0;
}

int
PVNewFilter::init(double *pp, int nargs)
{
	numgains = nargs;
	gains = new double[numgains+1];
	for (int arg = 0; arg < nargs; ++arg) {
		gains[arg] = pp[arg];
	}
	gains[nargs] = gains[nargs-1];	// one extra for interp
	return 1;
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
	int registerLib(int (*register_fun)(FilterCreateFunction)) {
		return register_fun(&PVNewFilter::create);
	}
}
