/* BUZZ - process a buzz wave signal with an IIR filter bank

   First, call setup to configure the filter bank:

      setup(cf1, bw1, gain1, cf2, bw2, gain2, ...)

	Each filter has a center frequency (cf), bandwidth (bw) and gain control.
	Frequency can be in Hz or oct.pc.  Bandwidth is in Hz, or if negative,
	is a multiplier of the center frequency.  Gain is the amplitude of this
	filter relative to the other filters in the bank.  There can be as many
	as 64 filters in the bank.

	Then call BUZZ:

      p0 = output start time
      p1 = duration
      p2 = amplitude
      p3 = pitch (Hz or oct.pc)
      p4 = pan (in percent-to-left form: 0-1) [optional, default is 0] 

   p2 (amplitude), p3 (pitch) and p4 (pan) can receive dynamic updates
   from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p2 amplitude multiplier, even if the latter is dynamic.

   When changing pitch dynamically, be aware of the implications of the
   dual-format pitch specification.  If the values drop below 15, then
   they will be interpreted as oct.pc.  Also, if you gliss from 8.00 down
   to 7.00, you will not get what you intend, because, for example,
   8.00 - .01 is 7.99, which is a very high pitch.

   By default, BUZZ uses an internally generated sine wave to create the
   buzz waveform.  But to be backward compatible with old scores, BUZZ
   accepts a gen table in slot 2 -- i.e., makegen(2, 10, 1024, 1).  This
   table must have exactly 1024 values.
                                          rev. for v4.0 by JGG, 7/10/04
*/
#include <stdio.h>
#include <math.h>
#include <ugens.h>
#include <Instrument.h>
#include "BUZZ.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	extern float rsnetc[64][5],amp[64];  /* defined in cfuncs.c */
	extern int nresons;
}

BUZZ::BUZZ() : Instrument()
{
	our_sine_table = false;
	branch = 0;
}

BUZZ::~BUZZ()
{
	if (our_sine_table)
		delete [] sinetable;
}

// NOTE: Sine table must have exactly 1024 elements,
// because of buzz ugen limitation.  -JGG

double *makeSineTable(int size)
{
	double *table = new double[size];
	double incr = (M_PI * 2.0) / (double) size;
	double phs = 0.0;
	for (int i = 0; i < size; i++, phs += incr)
		table[i] = sin(phs);
	return table;
}

int BUZZ::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];

	int nsamps = rtsetoutput(outskip, dur, this);

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(SR, dur, lenamp, amptabs);
	}

	sinetable = floc(2);
	if (sinetable) {
		lensine = fsize(2);
		if (lensine != 1024)
			return die("BUZZ", "Wavetable must have exactly 1024 values.");
	}
	else {
		lensine = 1024;
		sinetable = makeSineTable(lensine);
		our_sine_table = true;
	}
	phase = 0.0;
	prevpitch = -1.0;		// force first update

	for (int i = 0; i < nresons; i++) {
		myrsnetc[i][0] = rsnetc[i][0];
		myrsnetc[i][1] = rsnetc[i][1];
		myrsnetc[i][2] = rsnetc[i][2];
		myrsnetc[i][3] = myrsnetc[i][4] = 0.0;
		myamp[i] = amp[i];
	}
	mynresons = nresons;

	skip = (int) (SR / (float) resetval);

	return nsamps;
}

inline void BUZZ::setpitch(float pitch)
{
	if (pitch < 15.0)
		si = cpspch(pitch) * (float) lensine / SR;
	else
		si = pitch * (float) lensine / SR;
	hn = (int) (0.5 / (si / (float) lensine));
}

int BUZZ::run()
{
	for (int i = 0; i < framesToRun(); i++)  {
		if (--branch <= 0) {
			double p[5];
			update(p, 5);
			oamp = p[2];
			if (amparr)
				oamp *= tablei(cursamp, amparr, amptabs);
			float pitch = p[3];
			if (pitch <= 0.0)
				pitch = 0.01;
			if (pitch != prevpitch) {
				setpitch(pitch);
				prevpitch = pitch;
			}
			spread = p[4];
			branch = skip;
		}

		float sig = buzz(1.0, si, hn, sinetable, &phase);

		float out[2];
		out[0] = 0.0;
		for (int j = 0; j < mynresons; j++) {
			float val = reson(sig, myrsnetc[j]);
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

Instrument *makeBUZZ()
{
	BUZZ *inst;

	inst = new BUZZ();
	inst->set_bus_config("BUZZ");

	return inst;
}

