/* MBLOWBOTL - the "BlowBotl" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0 - 1.0, I think...)
   p6 = percent of signal to left output channel [optional, default is .5]
   p7 = breath pressure table [optional]
      (must be normalized between 0.0 - 1.0)

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is breathPressure (amplitude) curve
         for the note.  If no setline or function table 1, uses flat
         amplitude curve.)
      p3 (frequency)
      p4 (noise amp)
      p7 (breath pressure table)
*/

#include <Stk.h>
#include <BlowBotl.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBLOWBOTL.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBLOWBOTL :: MBLOWBOTL() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MBLOWBOTL :: ~MBLOWBOTL()
{
	delete theBotl;
}

int MBLOWBOTL :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amp = p[2];
	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theBotl = new BlowBotl();
	noiseamp = p[4];
	theBotl->setNoise(p[4]);
	freq = p[3];
	theBotl->noteOn(p[3], p[5]);

	pctleft = n_args > 6 ? p[6] : 0.5;                /* default is .5 */

	return nSamps();
}

void MBLOWBOTL :: doupdate()
{
	double p[8];
	update(p, 8, kAmp | kFreq | kNoise | kPan | kBreathPress);

	// "amp" is now separate from the breath pressure.
	// breath pressure is controlled by makegen 1, or a table, or it is 1.0
	if (amp != p[2])
		amp = p[2];
	if (amptable)
		breathamp = theEnv->next(currentFrame());
	else if (nargs > 7)
		breathamp = p[7];
	else
		breathamp = 1.0;

	if (freq != p[3]) {
		theBotl->setFrequency(p[3]);
		freq = p[3];
	}

	if (noiseamp != p[4]) {
		theBotl->setNoise(p[4]);
		noiseamp = p[4];
	}

	if (nargs > 6) pctleft = p[6];
}

int MBLOWBOTL :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}
		
		out[0] = theBotl->tick(breathamp) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBLOWBOTL()
{
	MBLOWBOTL *inst;

	inst = new MBLOWBOTL();
	inst->set_bus_config("MBLOWBOTL");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MBLOWBOTL", makeMBLOWBOTL);
}
#endif

