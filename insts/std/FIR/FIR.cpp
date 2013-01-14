/* FIR: simple FIR filter instrument
*
*  p0 = outsk
*  p1 = insk
*  p2 = dur
*  p3 = amp
*  p4 = total number of coefficients
*  p5...  the coefficients (up to 99 fir coefficients)
*
*  p3 (amp) can receive updates.
*  mono input / mono output only
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FIR.h"
#include <rt.h>
#include <rtdefs.h>

FIR::FIR() : Instrument()
{
	in = NULL;
	branch = 0;
}

FIR::~FIR()
{
	delete [] in;
}

int FIR::init(double p[], int n_args)
{
	if (rtsetinput(p[1], this) == -1)
		return DONT_SCHEDULE; // no input
	if (rtsetoutput(p[0], p[2], this) == -1)
		return DONT_SCHEDULE;

	ncoefs = (int)p[4];
	for (int i = 0; i < ncoefs; i++) {
		coefs[i] = p[i+6]; 
		pastsamps[i] = 0.0;
	}

	amp = p[3];

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int FIR::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

int FIR::run()
{
	float out[2];

	int rsamps = framesToRun() * inputChannels();
	rtgetin(in, this, rsamps);

	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			double p[4];
			update(p, 4, 1 << 3);
			amp = p[3];
			branch = skip;
		}
		out[0] = 0.0;
		pastsamps[0] = in[i * inputChannels()];

		for (int j = 0; j < ncoefs; j++) 
			out[0] += (pastsamps[j] * coefs[j]);
		for (int j = ncoefs-1; j >= 1; j--)
			pastsamps[j] = pastsamps[j-1];

		out[0] *= amp;
		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument*
makeFIR()
{
	FIR *inst;

	inst = new FIR();
	inst->set_bus_config("FIR");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("FIR",makeFIR);
}
#endif
