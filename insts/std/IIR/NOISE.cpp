/* NOISE - process white noise with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

	Each filter has a center frequency (cf), bandwidth (bw) and gain control.
	Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
	is a multiplier of the center frequency.  Gain is the amplitude of this
	filter relative to the other filters in the bank.  There can be as many
	as 64 filters in the bank.

	Then call NOISE:

      p0 = output start time
      p1 = duration
      p2 = amplitude
      p3 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p2 (amplitude) and p3 (pan) can receive dynamic updates from a table
   or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p2 amplitude multiplier, even if the latter is dynamic.

                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "NOISE.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	extern float rsnetc[64][5],amp[64];  /* defined in cfuncs.c */
	extern int nresons;
}

NOISE::NOISE() : Instrument()
{
	branch = 0;
}

NOISE::~NOISE()
{
}


int NOISE::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];

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

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int NOISE::run()
{
	for (int i = 0; i < framesToRun(); i++)  {
		if (--branch <= 0) {
			double p[4];
			update(p, 4, kAmp | kPan);
			oamp = p[2];
			if (amparr)
				oamp *= tablei(cursamp, amparr, amptabs);
			spread = p[3];
			branch = skip;
		}

		float sig = rrand();

		float out[2];
		out[0] = 0.0;
		for (int j = 0; j < mynresons; j++) {
			float val = reson(sig, myrsnetc[j]);
			out[0] += val * myamp[j];
		}

		out[0] *= oamp;
		if (outputchans == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeNOISE()
{
	NOISE *inst;

	inst = new NOISE();
	inst->set_bus_config("NOISE");

	return inst;
}

