/* MBRASS - the "Brass" physical model instrument in Perry Cook/Gary Scavone's
	"stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = slide length (samps)
   p5 = lip filter (Hz)
   p6 = max pressure (0.0 - 1.0, I think...)
   p7 = percent of signal to left output channel [optional, default is .5]
   p8 = breath pressure table [optional]

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is breathPressure (amplitude) curve
         for the note.  If no setline or function table 1, uses flat
         amplitude curve.)
      p3 (frequency)
      p4 (slide length)
      p5 (lip filter)
      p7 (pan)
*/

#include <Stk.h>
#include <Brass.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBRASS.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBRASS :: MBRASS() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MBRASS :: ~MBRASS()
{
	delete theHorn;
}

int MBRASS :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theHorn = new Brass(50.0); // 50 Hz is default lowest frequency
	freq = p[3];
	theHorn->setFrequency(p[3]);
	theHorn->startBlowing(p[6], 0.0);
	slength = p[4];
	theHorn->setSlide((int)p[4]);
	lipfilt = p[5];
	theHorn->setLip(p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nSamps();
}

void MBRASS :: doupdate()
{
	double p[9];
	update(p, 9, kAmp | kFreq | kSlide | kLip | kPan | kBreathPress);

	// "amp" is now separate from the breath pressure.
	// breath pressure is controlled by makegen 1, or a table, or it is 1.0
	amp = p[2] * 10.0; // for some reason this needs normalizing...
	if (amptable)
		breathamp = theEnv->next(currentFrame());
	else if (nargs > 8)
		breathamp = p[8];
	else
		breathamp = 1.0;

	if (freq != p[3]) {
		theHorn->setFrequency(p[3]);
		freq = p[3];
	}

	if (slength != p[4]) {
		theHorn->setSlide((int)p[4]);
		slength = p[4];
	}

	if (lipfilt != p[5]) {
		theHorn->setLip(p[5]);
		lipfilt = p[5];
	}

	if (nargs > 7) pctleft = p[7];
}

int MBRASS :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}

		out[0] = theHorn->tick(breathamp) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBRASS()
{
	MBRASS *inst;

	inst = new MBRASS();
	inst->set_bus_config("MBRASS");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MBRASS", makeMBRASS);
}
#endif

