/* CRACKLE - chaotic noise generator

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = chaos parameter (0-1) [optional, default is 1]
   p4 = pan (in percent-to-left format) [optional, default is .5]

   p2 (amp), p3 (chaos), and p4 (pan) can receive updates from
   a table or real-time control source.

   Neil Thornock <neilthornock at gmail>, 11/2016; rev. 1/2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <cmath>
#include "CRACKLE.h"
#include <rt.h>
#include <rtdefs.h>

CRACKLE::CRACKLE()
	: branch(0)
{
}

CRACKLE::~CRACKLE()
{
}

int CRACKLE::init(double p[], int n_args)
{
	nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("CRACKLE", "Use mono or stereo output only.");

	x0 = rrand() * 0.1;
	x1 = 0;
	x2 = 0;
	x3 = 0;

	return nSamps();
}

int CRACKLE::configure()
{
	return 0;
}

void CRACKLE::doupdate()
{
	double p[nargs];
	update(p, nargs);

	amp = p[2];
	param = (nargs > 3) ? p[3] * 0.22 + 0.83 : 1.05;
	pan = (nargs > 4) ? p[4] : 0.5;
}

int CRACKLE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		x3 = x2;
		x2 = x1;
		x1 = x0;
		x0 = std::abs(0.03 - x2 + x3 - param * x1);

		float out[2];
		if (x0 > 0.3)
			x0 = 0.2;
		out[0] = x0 * amp * 5;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pan);
			out[0] *= pan;
		}
		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeCRACKLE()
{
	CRACKLE *inst = new CRACKLE();
	inst->set_bus_config("CRACKLE");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("CRACKLE", makeCRACKLE);
}
#endif
