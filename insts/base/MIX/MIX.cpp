/*	Mix inputs to outputs with global amplitude control.

		p0 = output start time
		p1 = input start time
		p2 = duration (-endtime)
		p3 = amplitude multiplier
		p4-n = channel mix maxtrix

	p3 (amplitude) can receive dynamic updates from a table or real-time
	control source.

	If an old-style gen table 1 is present, its values will be multiplied
	by the p3 amplitude multiplier, even if the latter is dynamic.

	rev for v4, JGG, 7/9/04
*/
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "MIX.h"


MIX::MIX() : Instrument(), in(NULL)
{
	branch = 0;
}

MIX::~MIX()
{
	delete [] in;
}

int MIX::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];

	if (dur < 0.0)
		dur = -dur - inskip;

	nsamps = rtsetoutput(outskip, dur, this);
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	for (int i = 0; i < inputChannels(); i++) {
		outchan[i] = (int) p[i + 4];
		if (outchan[i] + 1 > outputChannels())
			return die("MIX",
						"You wanted output channel %d, but have only specified "
						"%d output channels", outchan[i], outputChannels());
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, tabs);
	}

	skip = (int) (SR / (float) resetval);

	return nSamps();
}


int MIX::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}


int MIX::run()
{
	int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[4];
			update(p, 4, 1 << 3);
			amp = p[3];
			if (amptable)
				amp *= tablei(currentFrame(), amptable, tabs);
			branch = skip;
		}

		float out[MAXBUS];
		for (int j = 0; j < outputChannels(); j++) {
			out[j] = 0.0;
			for (int k = 0; k < inputChannels(); k++) {
				if (outchan[k] == j)
					out[j] += in[i+k] * amp;
			}
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}



Instrument*
makeMIX()
{
	MIX *inst;

	inst = new MIX();
	inst->set_bus_config("MIX");

	return inst;
}

void
rtprofile()
{
   RT_INTRO("MIX",makeMIX);
}
