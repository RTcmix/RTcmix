/* DUMP - control stream debugger instrument

   p0 = output start time
   p2 = duration
   p3 = amp

   p3 (amplitude) can receive dynamic updates from a table or real-time
   control source.  This is the parameter whose values print.

   John Gibson (johgibso at indiana dot edu), 11/25/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <ugens.h>
#include <Instrument.h>
#include "DUMP.h"
#include <rt.h>
#include <rtdefs.h>


DUMP::DUMP() : Instrument()
{
	branch = 0;
	amp = -DBL_MAX;
}


DUMP::~DUMP()
{
}


int DUMP::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("DUMP", "Output must be mono or stereo.");

	skip = (int) (SR / (float) resetval);

	return nSamps();
}


void DUMP::doupdate()
{
	double p[3];
	update(p, 3);

	if (p[2] != amp) {
		amp = p[2];

		advise("DUMP", "%f", amp);
	}
}


int DUMP::configure()
{
	return 0;
}


int DUMP::run()
{
	float out[2] = {0.0, 0.0};

	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = skip;
		}
		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeDUMP()
{
	DUMP *inst;

	inst = new DUMP();
	inst->set_bus_config("DUMP");

	return inst;
}


void rtprofile()
{
	RT_INTRO("DUMP", makeDUMP);
}

