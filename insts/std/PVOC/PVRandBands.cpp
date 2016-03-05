//
//  PVRandBands.cpp
//  RTcmix
//
//  Created by Douglas Scott on 9/16/15.
//
//

#include "PVRandBands.h"
#include <string.h>
#include <ugens.h>

PVFilter *
PVRandBands::create()
{
	return new PVRandBands;
}

PVRandBands::PVRandBands() : _binGains(NULL)
{
}

PVRandBands::~PVRandBands()
{
	delete [] _binGains;
}

int
PVRandBands::run(float *pvdata, int nvals)
{
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		pvdata[ampIdx] *= _binGains[n];
	}
	return 0;
}

// total_bins, bins_to_chose, min_bin, max_bin [, gain_scalar]

int
PVRandBands::init(double *pp, int nargs)
{
	if (nargs >= 4) {
		int maxBins = (int) pp[0];
		int nBins = (int) pp[1];
		int minBin = (int) pp[2];
		int maxBin = (int) pp[3];
		if (minBin < 0 || maxBin < minBin || maxBin >= maxBins) {
			return die(NULL, "Illegal argument values");
		}
		rtcmix_advise(NULL, "Selecting %d active bins between bin %d and bin %d",
					  nBins, minBin, maxBin);
		_binGains = new float[maxBins];
		memset(_binGains, 0, maxBins*sizeof(float));
		float gainScale = (nargs == 5) ? pp[4] : 1.0;
		if (gainScale != 1.0) {
			rtcmix_advise(NULL, "Bin gains scaled by %.2f", gainScale);
		}
		for (int b = 0; b < nBins; ++b) {
			int bin = minBin + ((rrand() + 1.0) * (maxBin - minBin - 1));
			_binGains[bin] = gainScale;
		}
		return 1;
	}
Usage:
	return die(NULL, "Usage: PVRandBends(total_bins, num_bins, min_bin, max_bin [,gain_scale])");
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
	int registerLib(int (*register_fun)(FilterCreateFunction)) {
		return register_fun(&PVRandBands::create);
	}
}
