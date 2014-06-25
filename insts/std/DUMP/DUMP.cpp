/* DUMP - control stream debugger instrument

   p0 = output start time
   p1 = duration
   p2 = amp
   p3 = fixed table [optional]

   p2 (amplitude) can receive dynamic updates from a table or real-time
   control source.  This is the parameter whose values print.

   However, if there is a p3 and it's a table (returns a double array),
   then the *entire* contents of this array are printed on every update,
   and p2 is not printed at all.  This is a lot of data, so make a small
   table, and use a low control rate.

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
	table = NULL;
	tablelen = 0;
}


DUMP::~DUMP()
{
}


int DUMP::init(double p[], int n_args)
{
	nargs = n_args;
	float outskip = p[0];
	float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("DUMP", "Output must be mono or stereo.");

	if (nargs > 3)
		table = (double *) getPFieldTable(3, &tablelen);
	
	skip = (int) (SR / (float) resetval);

	return nSamps();
}


void DUMP::doupdate()
{
	double p[nargs];
	update(p, nargs);

	if (nargs > 3) {
		printf("DUMP (frame=%d)..............................\n", currentFrame());
		for (int i = 0; i < tablelen; i++)
			printf("  [%d]\t%f\n", i, table[i]);
		printf("\n");
		fflush(stdout);
	}
	else if (p[2] != amp) {
		amp = p[2];

		// Print regardless of global print flag state, so we can debug 
		// problems caused by argument printing invoking PField::doubleValue.
		printf("DUMP (frame=%d):  amp=%f\n", currentFrame(), amp);
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

