/* AMINST - amplitude modulation synthesis

   p0 = output start time
   p1 = duration
   p2 = amplitude *
   p3 = carrier frequency (Hz)
   p4 = modulation frequency (Hz)
   p5 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p6 = modulator amplitude [optional; if missing, must use gen 2 **,
        or default to 1.0]
   p7 = reference to carrier wavetable [optional; if missing, must use
        gen 3 ***, or default to internal sine wave]
   p8 = reference to modulator wavetable [optional; if missing, must use
        gen 4 ****, or default to internal sine wave]

   p2 (amplitude), p3 (carrier freq), p4 (modulator freq) and p5 (pan) can
   receive dynamic updates from a table or real-time control source.

   ---

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p2 (amplitude), even if the latter is dynamic.

   ** If p6 is missing, you must use an old-style gen table 2 for the
   modulation envelope. [added default of 1.0, BGG 1/2012]

   *** If p7 is missing, you must use an old-style gen table 3 for the
   carrier waveform. [added default sine wave, BGG 1/2012]

   **** If p8 is missing, you must use an old-style gen table 4 for the
   modulator waveform. [added default sine wave, BGG 1/2012]


   Author unknown (probably Brad Garton); rev for v4, JGG, 7/22/04
   [yes it was me -- BGG]
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "AMINST.h"
#include <rt.h>
#include <rtdefs.h>


AMINST::AMINST() : Instrument()
{
	carosc = NULL;
	modosc = NULL;
	branch = 0;
	ownModtable = false;
	ownCartable = false;
}

AMINST::~AMINST()
{
	delete carosc;
	delete modosc;
	if (ownModtable)
		delete [] modtable;
	if (ownCartable)
		delete [] cartable;
}

int AMINST::init(double p[], int n_args)
{
	float outskip = p[0];
	float dur = p[1];
	float carfreq = p[3];
	float modfreq = p[4];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("AMINST", "Can't handle more than 2 output channels.");

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(SR, dur, lenamp, amptabs);
	}

	modamparr = NULL;
	p6present = true; // but maybe it isn't...
	if (n_args < 7) { // no p6 mod amp PField, use gen table or default [1.0]
		p6present = false;
		modamparr = floc(2);
		if (modamparr == NULL) { // will default to 1.0 in doupdate()
			rtcmix_advise("AMINST", "no modulator amp (p6) present, defaulting to 1.0");
		} else {
			int len = fsize(2);
			tableset(SR, dur, len, modamptabs);
		}
	}

	cartable = NULL;
	int tablelen = 0;
	if (n_args > 7) {      // handle table coming in as optional p7 TablePField
		cartable = (double *) getPFieldTable(7, &tablelen);
	}
	if (cartable == NULL) {
		cartable = floc(3);
		if (cartable)
			tablelen = fsize(3);
		else {
			rtcmix_advise("AMINST", "No carrier wavetable specified, so using sine wave.");
			tablelen = 1024;
			cartable = new double [tablelen];
			ownCartable = true;
			const double twopi = M_PI * 2.0;
			for (int i = 0; i < tablelen; i++)
				cartable[i] = sin(twopi * ((double) i / tablelen));
		}
	}
	carosc = new Ooscili(SR, carfreq, cartable, tablelen);

	modtable = NULL;
	tablelen = 0;
	if (n_args > 8) {      // handle table coming in as optional p8 TablePField
		modtable = (double *) getPFieldTable(8, &tablelen);
	}
	if (modtable == NULL) {
		modtable = floc(4);
		if (modtable)
			tablelen = fsize(4);
		else {
			rtcmix_advise("AMINST", "No modulator wavetable specified, so using sine wave.");
			tablelen = 1024;
			modtable = new double [tablelen];
			ownModtable = true;
			const double twopi = M_PI * 2.0;
			for (int i = 0; i < tablelen; i++)
				modtable[i] = sin(twopi * ((double) i / tablelen));
		}
	}
	modosc = new Ooscili(SR, modfreq, modtable, tablelen);

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

void AMINST::doupdate()
{
	double p[7];
	update(p, 7);

	amp = p[2];
	if (amparr)
		amp *= tablei(currentFrame(), amparr, amptabs);

	float carfreq = p[3];
	carosc->setfreq(carfreq);

	float modfreq = p[4];
	modosc->setfreq(modfreq);

	spread = p[5];

	if (!p6present) {
		if (modamparr)
			modamp = tablei(currentFrame(), modamparr, modamptabs);
		else
			modamp = 1.0; // default mod amp
	}
	else
		modamp = p[6];
}

int AMINST::run()
{
	for (int i = 0; i < framesToRun(); i++)  {
		if (--branch <= 0) {
			doupdate();
			branch = skip;
		}

		float carsig = carosc->next();
		float outsig = carsig * modosc->next();

		float out[2];
		out[0] = (carsig * (1.0 - modamp)) + (outsig * modamp);
		out[0] *= amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeAMINST()
{
	AMINST *inst;

	inst = new AMINST();
	inst->set_bus_config("AMINST");

	return inst;
}

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("AMINST",makeAMINST);
}
#endif
