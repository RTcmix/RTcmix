/* DELAY - delay instrument with feedback

      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = delay time
      p5 = delay feedback (i.e., regeneration multiplier) [0-1]
      p6 = ring-down duration
      p7 = input channel [optional, default is 0]
      p8 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p3 (amplitude), p4 (delay time), p5 (feedback) and p8 (pan) can receive
   dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The point of the ring-down duration parameter is to let you control
   how long the delay will sound after the input has stopped.  Too short
   a time, and the sound may be cut off prematurely.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "DELAY.h"
#include <rt.h>
#include <rtdefs.h>

inline long max(long x, long y) { return x > y ? x : y; }

DELAY::DELAY() : Instrument()
{
	in = NULL;
	delay = NULL;
	branch = 0;
}

DELAY::~DELAY()
{
	delete [] in;
	delete delay;
}

int DELAY::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	float deltime = p[4];
	float ringdur = p[6];
	inchan = n_args > 7 ? (int) p[7] : 0;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (inchan >= inputChannels())
		return die("DELAY", "You asked for channel %d of a %d-channel file.",
												inchan, inputChannels());

	if (rtsetoutput(outskip, dur + ringdur, this) == -1)
		return DONT_SCHEDULE;
	insamps = (int) (dur * SR + 0.5);

	if (deltime <= 0.0)
		return die("DELAY", "Invalid delay time (%g)", deltime);

	long defaultDelay = max(100L, long(deltime * SR + 0.5));
	delay = new Odelayi(defaultDelay);
	// This is how we check for memory failure.
	if (delay->length() == 0)
		return die("DELAY", "Can't allocate delay line memory.");

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

	return nSamps();
}

int DELAY::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1; }

int DELAY::run()
{
	int samps = framesToRun() * inputChannels();

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[9];
			update(p, 9, kDelTime | kDelRegen | kPan);
			amp = update(3, insamps);
			if (amptable)
				amp *= tablei(cursamp, amptable, amptabs);
			float deltime = p[4];
			delsamps = deltime * SR;
			regen = p[5];
			pctleft = p[8];
			branch = getSkip();
		}

		float sig, out[2];

		if (currentFrame() < insamps)
			sig = in[i + inchan] * amp;
		else
			sig = 0.0;

		out[0] = sig + (delay->getsamp(delsamps) * regen);
		delay->putsamp(out[0]);

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makeDELAY()
{
	DELAY *inst;

	inst = new DELAY();
	inst->set_bus_config("DELAY");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("DELAY",makeDELAY);
}
#endif
