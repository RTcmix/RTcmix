/* MMODALBAR - the "ModalBar" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = stick hardness (0.0-1.0)
   p5 = stick position (0.0-1.0)
   p6 = modal preset
	- Marimba = 0
	- Vibraphone = 1
	- Agogo = 2
	- Wood1 = 3
	- Reso = 4
	- Wood2 = 5
	- Beats = 6
	- Two Fixed = 7
	- Clump = 8
   p7 = percent of signal to left output channel [optional, default is .5]
   p8 = amplitude envelope table [optional]

   PField updating:
      p2 (amplitude)
         (or assumes function table 1 is the amplitude curve for the note.
         If no setline or function table 1, uses flat amplitude curve.
         A table-handle in p8 may also be used.)
      p3 (frequency)
      p7 (pan)
		p8 (amplitude envelope table)
*/

#include <Stk.h>
#include <ModalBar.h> // from the stk library

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MMODALBAR.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>



MMODALBAR :: MMODALBAR() : Instrument()
{
	branch = 0;
	amptable = NULL;
}

MMODALBAR :: ~MMODALBAR()
{
//	delete [] excite;
	delete theFilt;
	delete theBar;
}

int MMODALBAR :: init(double p[], int n_args)
{
	int i;

	nargs = n_args;
	Stk::setSampleRate(SR);

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	amptable = floc(1);
	if (amptable) // the amp array has been created using makegen
		theEnv = new Ooscili(SR, 1.0/p[1], 1);

	theBar = new ModalBar();

	// create the excitation function -- I'm using the "marmstk1.raw"
	// file in perry/gary's stk as a very rough guide for this
	theRand = new Orand();
	for (i = 0; i < 25; i++)
		excite[i] = theRand->rand() * (float)i/25.0;
	for (i = 25; i < 256; i++)
		excite[i] = theRand->rand() * ( (231.0 - (float)(i-25))/231.0 );

	theFilt = new BiQuad();
	theFilt->setResonance(p[4]*100.0, 1.0-p[4], true);
	for (i = 0; i < 256; i++) excite[i] = theFilt->tick(excite[i]);

	theBar->setPreset((int)p[6]);
	theBar->setStickHardness(p[4]);
	theBar->setStrikePosition(p[5]);

	freq = p[3];
	// not sure if this normalizes the amplitude enough to compensate
	// for the filtering.  oh well.
	theBar->noteOn(p[3], p[4]); // amplitude handled by p[2] in this one

	pctleft = n_args > 7 ? p[7] : 0.5;                /* default is .5 */

	return nSamps();
}

void MMODALBAR :: doupdate()
{
	double p[9];
	update(p, 9, kAmp | kFreq | kPan | kAmpEnv);

	// "amp" is now separate from the excitation amp
	// excitation amp is controlled by makegen 1, or a table, or it is 1.0
	amp = p[2];
	if (amptable)
		exciteamp = theEnv->next(currentFrame());
	else if (nargs > 8)
		exciteamp = p[8];
	else 
		exciteamp = 1.0;

	if (freq != p[3]) {
		theBar->setFrequency(p[3]);
		freq = p[3];
	}

	if (nargs > 7) pctleft = p[7];
}

int MMODALBAR :: run()
{
	int   i;
	float out[2];

	for (i = 0; i < framesToRun(); i++) {
		if (--branch <=0) {
			doupdate();
			branch = getSkip();
		}

		if (currentFrame() < 256) // feed in excitation
			out[0] = theBar->tick(exciteamp, excite[currentFrame()]) * amp;
		else
			out[0] = theBar->tick(exciteamp, 0.0) * amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMMODALBAR()
{
	MMODALBAR *inst;

	inst = new MMODALBAR();
	inst->set_bus_config("MMODALBAR");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("MMODALBAR", makeMMODALBAR);
}
#endif

