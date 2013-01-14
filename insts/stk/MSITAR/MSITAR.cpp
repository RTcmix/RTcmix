/* MSITAR - the "Sitar" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = pluck amp (0.0-1.0)
   p5 = percent of signal to left output channel [optional, default is .5]
   p6 = amplitude table [optional]

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is string amplitude curve for the
         note.  Essentially this is just a 0.0-1.0 function that operates
         inside the stklib.  If no setline or function table 1, uses flat
         amplitude curve.  A table-handle in p6 may also be used)
      p3 (frequency)
      p5 (pan)
      p6 (amplitude table)
*/

#include <Stk.h>
#include <Sitar.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MSITAR.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MSITAR :: MSITAR() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MSITAR :: ~MSITAR()
{
	delete theSitar;
}

int MSITAR :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theSitar = new Sitar(50.0); // 50 Hz is lowest freq for now
	freq = p[3];
	theSitar->noteOn(p[3], p[4]);

	pctleft = n_args > 5 ? p[5] : 0.5;                /* default is .5 */

	return nSamps();
}


int MSITAR :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			double p[7];
			update(p, 7, kAmp | kFreq | kPan | kStramp);

			// "amp" is now separate from the internal string amp
			// string amp is controlled by makegen 1, or a table, or it is 1.0
			amp = p[2];
			if (amptable)
				stramp = theEnv->next(currentFrame());
			else if (nargs > 6)
				stramp = p[6];
			else 
				stramp = 1.0;

			if (freq != p[3]) {
				theSitar->setFrequency(p[3]);
				freq = p[3];
			}

			if (nargs > 5) pctleft = p[5];

			branch = getSkip();
		}

		out[0] = theSitar->tick(stramp) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMSITAR()
{
	MSITAR *inst;

	inst = new MSITAR();
	inst->set_bus_config("MSITAR");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MSITAR", makeMSITAR);
}
#endif

