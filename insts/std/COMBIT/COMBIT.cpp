/* COMBIT - Apply a comb filter to the input stream.

		p0 = output start time
		p1 = input start time
		p2 = input duration
		p3 = amplitude multiplier
		p4 = frequency (cps)
		p5 = reverb time
		p6 = input channel [optional]
		p7 = pan (percent to left) [optional]
      p8 = ring-down duration [optional, default is first reverb time value]

	p3 (amplitude), p4 (frequency), p5 (reverb time) and p7 (pan) can receive
	dynamic updates from a table or real-time control source.

	If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The point of the ring-down duration parameter is to let you control
   how long the combs will ring after the input has stopped.  If the
   reverb time is constant, COMBIT will figure out the correct ring-down
   duration for you.  If the reverb time is dynamic, you must specify a
   ring-down duration if you want to ensure that your sound will not be
   cut off prematurely.
                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <COMBIT.h>
#include <rt.h>
#include <rtdefs.h>

#define MINFREQ 0.1	// Hz

COMBIT::COMBIT() : Instrument()
{
	in = NULL;
	branch = 0;
	delsamps = 0;
	give_minfreq_warning = true;
}

COMBIT::~COMBIT()
{
	delete [] in;
	delete comb;
}


int COMBIT::init(double p[], int n_args)
{
	float start = p[0];
	float inskip = p[1];
	float dur = p[2];
	frequency = p[4];
	rvbtime = p[5];
	inchan = n_args > 6 ? (int) p[6] : 0;
	pctleft = n_args > 7 ? p[7] : 0.0;
	float ringdur = n_args > 8 ? p[8] : rvbtime;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (inchan >= inputChannels())
		return die("COMBIT", "You asked for channel %d of a %d-channel file.", 
                                                    inchan, inputChannels());

	if (rtsetoutput(start, dur + ringdur, this) == -1)
		return DONT_SCHEDULE;
	insamps = (int) (dur * SR + 0.5);

	if (frequency <= 0.0)
		return die("COMBIT", "Invalid frequency value!");
	float loopt = 1.0 / frequency;
	comb = new Ocombi(SR, loopt, loopt, rvbtime);
	if (comb->frequency() == 0.0)
		return die("COMBIT", "Failed to allocate comb memory!");

   frequency = -1.0;    // force update in run()

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur + rvbtime, amplen, tabs);
	}

	skip = (int) (SR / (float) resetval);

	return nSamps();
}


int COMBIT::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}


int COMBIT::run()
{
	int samps = framesToRun() * inputChannels();

	if (currentFrame() < insamps)
		rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[8];
			update(p, 8);
			amp = p[3];
			if (amptable) {
#ifdef MAXMSP
				amp *= rtcmix_table(currentFrame(), amptable, tabs);
#else
				amp *= table(currentFrame(), amptable, tabs);
#endif
			}
			if (p[4] != frequency) {
				frequency = p[4];
				delsamps = (int) ((1.0 / frequency) * SR + 0.5);
			}
			if (p[5] != rvbtime) {
				rvbtime = p[5];
				comb->setReverbTime(rvbtime);
			}
			pctleft = p[7];
			branch = skip;
		}

		float insig, out[2];

		if (currentFrame() < insamps)
			insig = in[i + inchan];
		else
			insig = 0.0;
		out[0] = comb->next(insig, delsamps) * amp;
		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - pctleft);
			out[0] *= pctleft;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeCOMBIT()
{
	COMBIT *inst;

	inst = new COMBIT();
	inst->set_bus_config("COMBIT");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("COMBIT",makeCOMBIT);
}
#endif
