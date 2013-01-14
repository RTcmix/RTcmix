/* MULTICOMB - 4 simultaneous comb filters randomly chosen within
   a specified range (and spread across the stereo field)

      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = comb frequency range bottom
      p5 = comb frequency range top
      p6 = reverb time
      p7 = input channel [optional]
      p8 = ring-down duration [optional, default is first reverb time value]

   p3 (amplitude) and p6 (reverb time) can receive dynamic updates from a
   table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The point of the ring-down duration parameter is to let you control
   how long the combs will ring after the input has stopped.  If the
   reverb time is constant, MULTICOMB will figure out the correct ring-down
   duration for you.  If the reverb time is dynamic, you must specify a
   ring-down duration if you want to ensure that your sound will not be
   cut off prematurely.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include "MULTICOMB.h"
#include <rt.h>
#include <rtdefs.h>


MULTICOMB::MULTICOMB() : Instrument()
{
	in = NULL;
	for (int n = 0; n < NCOMBS; n++)
		comb[n] = NULL;	
	branch = 0;
}

MULTICOMB::~MULTICOMB()
{
	delete [] in;
	for (int n = 0; n < NCOMBS; n++)
		delete comb[n];	
}

int MULTICOMB::init(double p[], int n_args)
{
	float start = p[0];
	float inskip = p[1];
	float dur = p[2];
	float minfreq = p[4];
	float maxfreq = p[5];
	rvbtime = p[6];
	inchan = n_args > 7 ? (int) p[7] : 0;
	float ringdur = n_args > 8 ? p[8] : rvbtime;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (inchan >= inputChannels())
		return die("MULTICOMB", "You asked for channel %d of a %d-channel file.", 
                                                    inchan, inputChannels());

	if (rtsetoutput(start, dur + ringdur, this) == -1)
		return DONT_SCHEDULE;
	insamps = (int) (dur * SR + 0.5);

	if (outputChannels() != 2)
		return die("MULTICOMB", "Output must be stereo.");

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

	for (int j = 0; j < NCOMBS; j++) {
		float cfreq = minfreq + ((maxfreq - minfreq) * (rrand() + 2.0) / 2.0);
		rtcmix_advise("MULTICOMB", "comb number %d: %g Hz", j, cfreq);
		float loopt = 1.0 / cfreq;
		delsamps[j] = (int) (loopt * SR + 0.5);
		comb[j] = new Ocomb(SR, loopt, rvbtime);
		if (comb[j]->frequency() == 0.0)
			return die("MULTICOMB", "Comb delay allocation failed.");
		spread[j] = (float) j / (float) (NCOMBS - 1);
	}

	return nSamps();
}

int MULTICOMB::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

int MULTICOMB::run()
{
	int samps = framesToRun() * inputChannels();

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[7];
			update(p, 7, kAmp | kRvbTime);
			amp = p[3];
			if (amptable) {
#ifdef MAXMSP
				amp *= rtcmix_table(currentFrame(), amptable, amptabs);
#else
				amp *= table(currentFrame(), amptable, amptabs);
#endif
			}
			if (p[6] != rvbtime) {
				rvbtime = p[6];
				for (int j = 0; j < NCOMBS; j++)
					comb[j]->setReverbTime(rvbtime);
			}
			branch = getSkip();
		}

		float insig, out[2];

		if (currentFrame() < insamps)
			insig = in[i + inchan];
		else
			insig = 0.0;

		out[0] = out[1] = 0.0;
		for (int j = 0; j < NCOMBS; j++) {
			float sig = comb[j]->next(insig, delsamps[j]);
			out[0] += sig * spread[j]; 
			out[1] += sig * (1.0 - spread[j]);
		}

		out[0] *= amp;
		out[1] *= amp;

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeMULTICOMB()
{
	MULTICOMB *inst;

	inst = new MULTICOMB();
	inst->set_bus_config("MULTICOMB");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("MULTICOMB",makeMULTICOMB);
}
#endif
