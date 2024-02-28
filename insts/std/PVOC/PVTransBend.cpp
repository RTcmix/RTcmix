// PVTransBend.cpp

#include "PVTransBend.h"
#include <math.h>
#include <rtdefs.h>
#include <ugens.h>
#include <stdio.h>

extern "C" double gen5(struct gen *gen);	/* libgen */
static const int kArraySize = 2048;

PVFilter *
PVTransBend::create()
{
	return new PVTransBend;
}

PVTransBend::PVTransBend() : _exptable(0), _totalCalls(0), _currentCall(0)
{
}

PVTransBend::~PVTransBend()
{
	delete [] _exptable;
}

int
PVTransBend::run(float *pvdata, int nvals)
{
	int index = (int) ((float)kArraySize * _currentCall / _totalCalls);
	if (index > kArraySize-1) index = kArraySize-1;
	double factor = _exptable[index];
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		const int frqIdx = ampIdx + 1;
		pvdata[frqIdx] *= factor;
	}
	++_currentCall;
	return 0;
}

int
PVTransBend::init(double *pp, int nargs)
{
	_totalCalls = (int) pp[0];
	if (_totalCalls <= 0) {
		die("init_filter", "total_calls must be > 0");
        return PARAM_ERROR;
	}
	_currentCall = 0;
	_exptable = new double[2048];
	if (nargs >= 3) {
		float *pvals = new float[nargs - 1];
		float timeZero = pp[1];
		float totalTime = pp[nargs - 2], prevTime = timeZero;
		int pIndex = 0;
		for (int arg=1; arg<nargs; arg+=2) {
			if (pp[arg] < prevTime) {
                delete [] pvals;
				die("init_filter", "Time values must be in ascending order");
                return PARAM_ERROR;
			}
			pvals[pIndex] = (pp[arg] - timeZero) * (kArraySize-1) / totalTime;	// position
			pvals[pIndex+1] = cpsoct(10.0 + octpch(pp[arg+1])) / cpsoct(10.0);	// transp
			pIndex += 2;
		}
		struct gen gen;
		gen.size = 2048;
		gen.nargs = nargs-2;
		gen.pvals = &pvals[1];	// skip initial time value
		gen.array = _exptable;
		gen.slot = -1;	// don't rescale
		gen5(&gen);
		delete [] pvals;
		return 0;
	}
	die("init_filter", "PVTransBend usage: init_filter(total_calls, time0, intrvl0, ..., timeN, intrvlN)");
    return PARAM_ERROR;
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
int registerLib(int (*register_fun)(FilterCreateFunction)) {
	return register_fun(&PVTransBend::create);
}
}
