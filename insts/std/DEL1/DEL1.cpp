/* DEL1 - split mono source to two channels, delay right channel

      p0 = output start time
      p1 = input start time
      p2 = output duration
      p3 = amplitude multiplier
      p4 = right channel delay time
      p5 = right channel amplitude multiplier (relative to left channel)
      p6 = input channel [optional, default is 0]
      p7 = ring-down duration [optional, default is first delay time value] 

   p3 (amplitude), p4 (delay time) and p5 (delay amplitude) can receive
	dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

	The point of the ring-down duration parameter is to let you control
	how long the delay will sound after the input has stopped.  If the
	delay time is constant, DEL1 will figure out the correct ring-down
	duration for you.  If the delay time is dynamic, you must specify a
	ring-down duration if you want to ensure that your sound will not be
	cut off prematurely.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include "DEL1.h"
#include <rt.h>
#include <rtdefs.h>

#define MAXDELTIME 30.0		// seconds (176400 bytes per second at SR=44100

DEL1::DEL1() : Instrument()
{
	delay = NULL;
	in = NULL;
	branch = 0;
	warn_deltime = true;
}

DEL1::~DEL1()
{
	delete [] in;
	delete delay;
}

int DEL1::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	float deltime = p[4];
	inchan = n_args > 6 ? (int) p[6] : 0;
	float ringdur = n_args > 7 ? p[7] : deltime;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (inchan >= inputChannels())
		return die("DEL1", "You asked for channel %d of a %d-channel file.",
													inchan, inputChannels());

	if (rtsetoutput(outskip, dur + ringdur, this) == -1)
		return DONT_SCHEDULE;
	insamps = (int) (dur * SR + 0.5);

	if (outputChannels() != 2)
		return die("DEL1", "Output must be stereo.");

	if (deltime <= 0.0)
		return die("DEL1", "Illegal delay time (%g).", deltime);

	long delsamps = (long) (deltime * SR + 0.5);
	delay = new Odelayi(delsamps);
	if (delay->length() == 0)
		return die("DEL1", "Can't allocate delay line memory.");

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int DEL1::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

int DEL1::run()
{
	int samps = framesToRun() * inputChannels();

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[6];
			update(p, 6);
			amp = p[3];
			if (amptable)
				amp *= tablei(cursamp, amptable, amptabs);
			float deltime = p[4];
			delsamps = deltime * SR;
			delamp = p[5];
			branch = skip;
		}

		float sig;
		if (currentFrame() < insamps)
			sig = in[i + inchan];
		else
			sig = 0.0;

		float out[2];
		out[0] = sig * amp;
		out[1] = delay->getsamp(delsamps) * delamp;
		delay->putsamp(sig);

		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makeDEL1()
{
	DEL1 *inst;

	inst = new DEL1();
	inst->set_bus_config("DEL1");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("DEL1",makeDEL1);
}
#endif
