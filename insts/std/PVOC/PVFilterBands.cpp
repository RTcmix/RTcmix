//
//  PVFilterBands.cpp
//  RTcmix
//
//  Created by Douglas Scott on 9/16/15.
//
//

#include "PVFilterBands.h"
#include <string.h>
#include <ugens.h>

PVFilter *
PVFilterBands::create()
{
	return new PVFilterBands;
}

PVFilterBands::PVFilterBands() : _binGains(NULL)
{
}

PVFilterBands::~PVFilterBands()
{
	delete [] _binGains;
}

int
PVFilterBands::run(float *pvdata, int nvals)
{
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		pvdata[ampIdx] *= _binGains[n];
	}
	return 0;
}

// totalBins, bin0, gain0, bin1, gain1, ..., binN, gainN

int
PVFilterBands::init(double *pp, int nargs)
{
    int pvocBins = (int)pp[0];
    int numBins = nargs/2;
    _binGains = new float[pvocBins];
    memset(_binGains, 0, numBins*sizeof(float));
    int prevBin = 0;
    float prevGain = 1.0;
	for (int arg = 1; arg < nargs; arg += 2) {
        int bin0 = (int)pp[arg];
        double gain0 = pp[arg+1];
		if (bin0 < prevBin || bin0 >= pvocBins) {
			return die(NULL, "Illegal argument values");
		}
        for (int pbin = prevBin; pbin < bin0; ++pbin) {
            _binGains[pbin] = (gain0-prevGain) * (float(pbin-prevBin)/(bin0-prevBin));
        }
        prevBin = bin0;
        prevGain = gain0;
	}
    // extend last provided gain through remaining bins
    for (int pbin = prevBin; pbin < pvocBins; ++pbin) {
        _binGains[pbin] = prevGain;
    }
    return 1;
Usage:
	return die(NULL, "Usage: PVFilterBands(fft_size, bin0, gain0, binX, gainX, binY, gainY, ...");
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
	int registerLib(int (*register_fun)(FilterCreateFunction)) {
		return register_fun(&PVFilterBands::create);
	}
}
