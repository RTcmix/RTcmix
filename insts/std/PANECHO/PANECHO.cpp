/* PANECHO - stereo panning delay instrument

      p0 = output start time
      p1 = input start time
      p2 = input duration
      p3 = amplitude multiplier
      p4 = left channel delay time
      p5 = right channel delay time
      p6 = delay feedback (i.e., regeneration multiplier) [0-1]
      p7 = ring-down duration
      p8 = input channel [optional, default is 0]

   p3 (amplitude), p4 (left delay time), p5 (right delay time) and
   p6 (feedback) can receive dynamic updates from a table or real-time
   control source.

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
#include <Instrument.h>
#include "PANECHO.h"
#include <rt.h>
#include <rtdefs.h>

#define MAXDELTIME 20.0    // seconds (176400 bytes per second at SR=44100)

PANECHO::PANECHO() : Instrument()
{
	in = NULL;
	branch = 0;
	warn_deltime = true;
}

PANECHO::~PANECHO()
{
	delete [] in;
	delete delay0;
	delete delay1;
}

int PANECHO::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	float deltime0 = p[4];
	float deltime1 = p[5];
	float ringdur = p[7];
	inchan = n_args > 8 ? (int) p[8] : 0;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (inchan >= inputChannels())
		return die("PANECHO", "You asked for channel %d of a %d-channel file.",
                                                    inchan, inputChannels());

	nsamps = rtsetoutput(outskip, dur + ringdur, this);
	insamps = (int) (dur * SR + 0.5);

	if (outputChannels() != 2)
		return die("PANECHO", "Output must be stereo.");

	if (deltime0 > MAXDELTIME || deltime1 > MAXDELTIME)
		return die("PANECHO", "Maximum delay time (%g seconds) exceeded.",
                                                            MAXDELTIME);
	long maxdelsamps = (long) (MAXDELTIME * SR + 0.5);
	delay0 = new Odelayi(maxdelsamps);
	delay1 = new Odelayi(maxdelsamps);
	if (delay0 == NULL || delay1 == NULL)
		return die("PANECHO", "Can't allocate delay line memory.");
	prevdeltime0 = prevdeltime1 = -999999999.9;		// force first update

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, amptabs);
	}

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int PANECHO::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

inline double PANECHO::getdelsamps(float deltime)
{
	if (deltime > MAXDELTIME) {
		if (warn_deltime) {
			warn("PANECHO", "Maximum delay time (%g seconds) exceeded!",
                                                          MAXDELTIME);
			warn_deltime = false;
		}
		return (MAXDELTIME * SR);
	}
	return (deltime * SR);
}

int PANECHO::run()
{
	int samps = framesToRun() * inputChannels();

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[7];
			update(p, 7, kAmp | kDelTime0 | kDelTime1 | kDelRegen);
			amp = p[3];
			if (amptable)
				amp *= tablei(cursamp, amptable, amptabs);
			float thisdeltime = p[4];
			if (thisdeltime != prevdeltime0) {
				delsamps0 = getdelsamps(thisdeltime);
				prevdeltime0 = thisdeltime;
			}
			thisdeltime = p[5];
			if (thisdeltime != prevdeltime1) {
				delsamps1 = getdelsamps(thisdeltime);
				prevdeltime1 = thisdeltime;
			}
			regen = p[6];
			branch = skip;
		}

		float sig, out[2];

		if (currentFrame() < insamps)
			sig = in[i + inchan] * amp;
		else
			sig = 0.0;

		out[0] = sig + (delay1->getsamp(delsamps1) * regen);
		out[1] = delay0->getsamp(delsamps0);

		delay0->putsamp(out[0]);
		delay1->putsamp(out[1]);

		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makePANECHO()
{
	PANECHO *inst;

	inst = new PANECHO();
	inst->set_bus_config("PANECHO");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("PANECHO",makePANECHO);
}

