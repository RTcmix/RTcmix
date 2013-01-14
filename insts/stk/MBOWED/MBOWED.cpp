/* MBOWED - the "Bowed" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = vibrato freq low (Hz)
   p5 = vibrato freq high (Hz)
   p6 = vibrato depth (% of base frequency [decimal notation 0.1 == 10%])
   p7 = percent of signal to left output channel [optional, default is .5]
   p8 = bow velocity (amplitude) table [optional] (0.0-1.0)
   p9 = bow pressure table [optional] (0.0-1.0)
   p10 = bow position table [optional] (0.0-1.0)
   p11 = vibrato wavetable [optional]

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is bow velocity (amplitude) curve
         for the note.  If no setline or function table 1, uses flat (1.0)
         amplitude curve.  p8 can also contain a PField ref for this)
      p3 (frequency)
      p6 (vibrato depth)
      p7 (pan)
      p8 (bow velocity table) [function 1 should be present if unused]
      p9 (bow pressure table) [function 2 should be present if unused]
      p10 (bow position table) [function 3 should be present if unused]
      p11 (vibrato wavetable) [function 4 should be present if unused]
*/

#include <Stk.h>
#include <Bowed.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MBOWED.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MBOWED :: MBOWED() : Instrument()
{
	branch = 0;
	amptable = NULL;
	vibtable = NULL;
}

MBOWED :: ~MBOWED()
{
	delete theBow;
}

int MBOWED :: init(double p[], int n_args)
{
	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theRand = new Orand();

	if (n_args < 10)
		thePressure = new Ooscili(SR, 1.0/p[1], 2);

	if (n_args < 11)
		thePosition = new Ooscili(SR, 1.0/p[1], 3);

	viblo = p[4];
	vibhi = p[5];

	int vtablelen = 0;
	if (n_args > 11) // if vibrato waveform is p11 table-handle
		vibtable = (double *) getPFieldTable(11, &vtablelen);
	if (vibtable == NULL) {
		vibtable = floc(4);
		if (vibtable == NULL)
			return die("MBOWED", "no vibrato waveform in function slot 4 or p11");
		vtablelen = fsize(4);
	}
	theVib = new Ooscili(SR, theRand->range(viblo, vibhi), vibtable, vtablelen);
	vibupdate = 0;

	freqbase = p[3] - (p[6] * p[3]);
	freqamp = 2.0 * (p[6] * p[3]);

	theBow = new Bowed(50.0); // 50 Hz is lowest freq for now

	theBow->noteOn(p[3], p[5]);

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nSamps();
}

void MBOWED:: doupdate()
{
	double p[11];
	update(p, 11, kAmp | kFreq | kVibDepth | kPan | kBowVel | kBowPress | kBowPos);

	// "amp" is now separate from the bow velocity
	// bow velocity is controlled by makegen 1, or a table, or it is 1.0
	amp = p[2] * 3.0; // needs normalizing
	if (amptable)
		bowvel = theEnv->next(currentFrame());
	else if (nargs > 8)
		bowvel = p[8];
	else
		bowvel = 1.0;

	freqbase = p[3] - (p[6] * p[3]);
	freqamp = 2.0 * (p[6] * p[3]);

	if (nargs > 7) pctleft = p[7];

	if (nargs > 9)
		theBow->setBowPressure(p[9]);
	else
		theBow->setBowPressure(thePressure->next(currentFrame()));

	if (nargs > 10)
		theBow->setBowPosition(p[10]);
	else
		theBow->setBowPosition(thePosition->next(currentFrame()));
}

int MBOWED :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <= 0) {
			doupdate();
			branch = getSkip();
		}
		if (--vibupdate < 0) { // reset the vibrato freq after each cycle
			float vibfreq = theRand->range(viblo, vibhi);
			theVib->setfreq(vibfreq);
			vibupdate = (int)(SR/vibfreq);
		}

		out[0] = theBow->tick(bowvel) * amp;
		theBow->setFrequency(freqbase + (freqamp * ( (theVib->next()+2.0) * 0.5 )));

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMBOWED()
{
	MBOWED *inst;

	inst = new MBOWED();
	inst->set_bus_config("MBOWED");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MBOWED", makeMBOWED);
}
#endif

