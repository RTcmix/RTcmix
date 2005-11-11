// PVRandBend.cpp

#include "PVRandBend.h"
#include <ugens.h>

static const double kDefaultPitchRange = 100.0;

PVFilter *
PVRandBend::create()
{
	return new PVRandBend;
}

PVRandBend::PVRandBend() : _pitchRange(kDefaultPitchRange), _bins(0)
{
}

PVRandBend::~PVRandBend()
{
	delete [] _bins;
}

int
PVRandBend::run(float *pvdata, int nvals)
{
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		const int frqIdx = ampIdx + 1;
		Bin *b = &_bins[n];
		const double incr = b->incr;
		pvdata[frqIdx] += b->val;
		b->val += incr;
		if (incr < 0.0) {
			if (b->val < b->newval) {
				b->newval = rrand() * _pitchRange / 2.0;
				b->incr = rrand() * _pitchRange / 32;
				if (b->val > b->newval)
					b->incr *= -1.0;
			}
		}
		else {
			if (b->val > b->newval) {
				b->newval = rrand() * _pitchRange / 2.0;
				b->incr = rrand() * _pitchRange / 32;
				if (b->val < b->newval)
					b->incr *= -1.0;
			}
		}
	}
	return 0;
}

int
PVRandBend::init(double *pp, int nargs)
{
	int nBins = (int) pp[0];
	_bins = new Bin[nBins];
	if (nargs > 1)
		_pitchRange = pp[1];
	for (int n = 0; n < nBins; ++n) {
		_bins[n].val = 0.0;
		_bins[n].newval = rrand() * _pitchRange / 2.0;
		_bins[n].incr = rrand() * _pitchRange / 32;
	}
	return 1;
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
int registerLib(int (*register_fun)(FilterCreateFunction)) {
	return register_fun(&PVRandBend::create);
}
}
