/* Apply a comb filter to the input stream.

		p0 = output start time
		p1 = input start time
		p2 = input duration
		p3 = amplitude multiplier
		p4 = frequency (cps)
		p5 = reverb time
		p6 = input channel [optional]
		p7 = pan (percent to left) [optional]

	p3 (amplitude), p4 (frequency), p5 (reverb time) and p7 (pan) can receive
	dynamic updates from a table or real-time control source.

	If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.
*/
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "COMBIT.h"
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

	int rvin = rtsetinput(inskip, this);
	if (rvin == -1) { // no input
		return DONT_SCHEDULE;
	}
	if (inchan >= inputChannels())
		return die("COMBIT", "You asked for channel %d of a %d-channel file.", 
                                                    inchan, inputChannels());
// FIXME: if update of rvbtime causes its value to increase, output will
// be cut short.  Solutions: (1) as user, put big value in first table slot;
// (2) here, pass bigger value to rtsetoutput.  (1) is confusing, (2) could
// give way too much ringdur.  Probably should be an optional ringdur pfield.
	nsamps = rtsetoutput(start, dur + rvbtime, this);
	insamps = (int) (dur * SR + 0.5);

	if (frequency < MINFREQ)
		return die("COMBIT", "Invalid frequency value!");
	float loopt = 1.0 / frequency;
	comb = new Ocomb(loopt, 1.0 / MINFREQ, rvbtime);
   frequency = -1.0;    // force update in run()

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(dur + rvbtime, amplen, tabs);
	}

	skip = (int) (SR / (float) resetval);

	return nsamps;
}


int COMBIT::configure()
{
	if (in == NULL)
		in = new float [RTBUFSAMPS * inputChannels()];

	return 0;	// return 0 on success, -1 on failure
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
			if (amptable)
				amp *= table(currentFrame(), amptable, tabs);
			if (p[4] != frequency) {
				if (p[4] < MINFREQ) {
					if (give_minfreq_warning) {
						warn("COMBIT", "Invalid frequency value!");
						give_minfreq_warning = false;
					}
				}
				else {
					frequency = p[4];
					delsamps = (int) ((1.0 / frequency) * SR + 0.5);
				}
			}
			if (p[5] != rvbtime) {
				rvbtime = p[5];
				comb->setReverbTime(rvbtime);
			}
			inchan = (int) p[6];
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


Instrument*
makeCOMBIT()
{
	COMBIT *inst;

	inst = new COMBIT();
	inst->set_bus_config("COMBIT");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("COMBIT",makeCOMBIT);
}
