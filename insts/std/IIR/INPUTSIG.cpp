/* INPUTSIG - process a mono input with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

	Each filter has a center frequency (cf), bandwidth (bw) and gain control.
	Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
	is a multiplier of the center frequency.  Gain is the amplitude of this
	filter relative to the other filters in the bank.  There can be as many
	as 64 filters in the bank.

	Then call INPUTSIG:

      p0 = output start time
      p1 = input start time
      p2 = duration
      p3 = amplitude multiplier
      p4 = input channel [optional, default is 0]
      p5 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p3 (amplitude) and p5 (pan) can receive dynamic updates from a table
   or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include "INPUTSIG.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	extern float rsnetc[64][5],amp[64];  /* defined in cfuncs.c */
	extern int nresons;
}

INPUTSIG::INPUTSIG() : Instrument()
{
	in = NULL;
	branch = 0;
}

INPUTSIG::~INPUTSIG()
{
	delete [] in;
}


int INPUTSIG::init(double p[], int n_args)
{
	float outskip = p[0];
	float inskip = p[1];
	float dur = p[2];
	inchan = n_args > 4 ? (int) p[4] : 0;

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(SR, dur, lenamp, amptabs);
	}

	for (int i = 0; i < nresons; i++) {
		myrsnetc[i][0] = rsnetc[i][0];
		myrsnetc[i][1] = rsnetc[i][1];
		myrsnetc[i][2] = rsnetc[i][2];
		myrsnetc[i][3] = myrsnetc[i][4] = 0.0;
		myamp[i] = amp[i];
	}
	mynresons = nresons;

	if (inchan >= inputChannels())
		return die("INPUTSIG", "You asked for channel %d of a %d-channel file.",
														inchan, inputChannels());

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int INPUTSIG::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}

int INPUTSIG::run()
{
	int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[6];
			update(p, 6, kAmp | kPan);
			oamp = p[3];
			if (amparr)
				oamp *= tablei(cursamp, amparr, amptabs);
			spread = p[5];
			branch = skip;
		}

		float out[2];
		out[0] = 0.0;
		for (int j = 0; j < mynresons; j++) {
			float val = reson(in[i+inchan], myrsnetc[j]);
			out[0] += val * myamp[j];
		}

		out[0] *= oamp;
		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeINPUTSIG()
{
	INPUTSIG *inst;

	inst = new INPUTSIG();
	inst->set_bus_config("INPUTSIG");

	return inst;
}

