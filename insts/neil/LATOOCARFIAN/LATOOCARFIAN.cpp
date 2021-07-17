/* LATOOCARFIAN - chaotic noise generator

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = parameter a [optional, default is 2.871]
   p4 = parameter b [optional, default is 2.75]
   p5 = parameter c [optional, default is 1]
   p6 = parameter d [optional, default is 0.75]
   p7 = seed x [optional, default is 0.5]
   p8 = seed y [optional, default is 0.5]
   p9 = pan (in percent-to-left format) [optional, default is .5]

   p2 (amp), p3-p8 (function parameters), and p9 (pan) can receive
   updates from a table or real-time control source.

   Any values for p3-p8 are legal. Pickover recommends values for
   a and b between -3 and 3, and p4 and p5 between 0.5 and 1.5.
   Depending on the values provided, results may be chaotic noise,
   pitch, or silence.

   Neil Thornock <neilthornock at gmail>, 11/2016

   Based on a function given in Clifford Pickover's book Chaos
   in Wonderland.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include "LATOOCARFIAN.h"
#include <rt.h>
#include <rtdefs.h>

LATOOCARFIAN::LATOOCARFIAN()
	: branch(0)
{
}

LATOOCARFIAN::~LATOOCARFIAN()
{
}

int LATOOCARFIAN::init(double p[], int n_args)
{
	nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];

	x = (nargs > 7) ? p[7] : 0.5;
	y = (nargs > 8) ? p[8] : 0.5;
	prevx = x;

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("LATOOCARFIAN", "Use mono or stereo output only.");

	return nSamps();
}

int LATOOCARFIAN::configure()
{
	return 0;
}

void LATOOCARFIAN::doupdate()
{
	// BGGx ww -- arg!
	//double p[nargs];
	double *p = new double[nargs];
	update(p, nargs);

	amp = p[2];
	param1 = (nargs > 3) ? p[3] : 2.871;
	param2 = (nargs > 4) ? p[4] : 2.75;
	param3 = (nargs > 5) ? p[5] : 1;
	param4 = (nargs > 6) ? p[6] : 0.75;

	pan = (nargs > 9) ? p[9] : 0.5;
}

int LATOOCARFIAN::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		x = sin(param2 * y) + param3 * sin(param2 * x);
		y = sin(param1 * prevx) + param4 * sin(param1 * y);
		prevx = x;

		float samp;
		if (x > 1 || x < -1)
			samp = sin(x);
		else
			samp = x;

		float out[2];
		out[0] = samp * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pan);
			out[0] *= pan;
		}
		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeLATOOCARFIAN()
{
	LATOOCARFIAN *inst = new LATOOCARFIAN();
	inst->set_bus_config("LATOOCARFIAN");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("LATOOCARFIAN", makeLATOOCARFIAN);
}
#endif
