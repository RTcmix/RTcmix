// PVFilterTest.cpp -- test class for pvoc data filters

#include "PVFilterTest.h"
#include <stdio.h>

PVFilter *
PVFilterTest::create()
{
	return new PVFilterTest;
}

PVFilterTest::PVFilterTest()
{
}

PVFilterTest::~PVFilterTest()
{
}

int
PVFilterTest::run(float *pvdata, int nvals)
{
	for (int n = 0; n < nvals; ++n) {
		const int ampIdx = n * 2;
		const int frqIdx = ampIdx + 1;
		pvdata[frqIdx] *= val2;
		pvdata[frqIdx] += val1;
	}
	return 0;
}

int
PVFilterTest::init(double *pp, int nargs)
{
	val1 = pp[0];
	val2 = pp[1];
	return 1;
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
int registerLib(int (*register_fun)(FilterCreateFunction)) {
	return register_fun(&PVFilterTest::create);
}
}
