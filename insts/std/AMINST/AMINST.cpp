/* AMINST - amplitude modulation synthesis

   p0 = output start time
   p1 = duration
   p2 = amplitude *
   p3 = carrier frequency (Hz)
   p4 = modulation frequency (Hz)
   p5 = pan (in percent-to-left form: 0-1) [optional; default is 0]
   p6 = modulator amplitude [optional; if missing,must use gen 2 **]
   p7 = reference to carrier wavetable [optional; if missing, must use
        gen 3 ***]
   p8 = reference to modulator wavetable [optional; if missing, must use
        gen 4 ****]

   p2 (amplitude), p3 (carrier freq), p4 (modulator freq) and p5 (pan) can
   receive dynamic updates from a table or real-time control source.

   ---

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p2 (amplitude), even if the latter is dynamic.

   ** If p6 is missing, you must use an old-style gen table 2 for the
   modulation envelope.

   *** If p7 is missing, you must use an old-style gen table 3 for the
   carrier waveform.

   **** If p8 is missing, you must use an old-style gen table 4 for the
   modulator waveform.


   Author unknown (probably Brad Garton); rev for v4, JGG, 7/22/04
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
}

AMINST::~AMINST()
{
	delete carosc;
	delete modosc;
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
	if (n_args < 7) {      // no p6 mod amp PField, must use gen table
		modamparr = floc(2);
		if (modamparr == NULL)
			return die("AMINST", "Either use the mod. amp pfield (p6) or "
							"make an old-style gen function in slot 2.");
		int len = fsize(2);
		tableset(SR, dur, len, modamptabs);
	}

	cartable = NULL;
	int tablelen = 0;
	if (n_args > 7) {      // handle table coming in as optional p7 TablePField
		cartable = (double *) getPFieldTable(7, &tablelen);
	}
	if (cartable == NULL) {
		cartable = floc(3);
		if (cartable == NULL)
			return die("AMINST", "Either use the carrier wavetable pfield (p7) "
						"or make an old-style gen function in slot 3.");
		tablelen = fsize(3);
	}
	carosc = new Ooscili(SR, carfreq, cartable, tablelen);

	modtable = NULL;
	tablelen = 0;
	if (n_args > 8) {      // handle table coming in as optional p8 TablePField
		modtable = (double *) getPFieldTable(8, &tablelen);
	}
	if (modtable == NULL) {
		modtable = floc(4);
		if (modtable == NULL)
			return die("AMINST", "Either use the modulator wavetable pfield (p8) "
						"or make an old-style gen function in slot 4.");
		tablelen = fsize(4);
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

	if (modamparr)
		modamp = tablei(currentFrame(), modamparr, modamptabs);
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

void
rtprofile()
{
	RT_INTRO("AMINST",makeAMINST);
}

