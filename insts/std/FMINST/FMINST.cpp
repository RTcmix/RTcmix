/* FMINST -- simple fm instrument
*
*  p0 = start time
*  p1 = duration
*  p2 = amp
*  p3 = pitch of carrier (hz or oct.pc)
*  p4 = pitch of modulator (hz or oct.pc)
*  p5 = fm index low point
*  p6 = fm index high point
*  p7 = stereo spread (0-1) <optional>
*  function slot 1 is amp envelope, slot 2 is oscillator waveform, 
*     slot 3 is index guide
*  [this instrument has real-time control of p2, p3, p4 and p6 enabled]
*/
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FMINST.h"
#include <rt.h>
#include <rtdefs.h>
#include <notetags.h>

#ifdef COMPATIBLE_FUNC_LOCS         /* set in makefile.conf */
  #define AMP_GEN_SLOT     2
  #define WAVET_GEN_SLOT   1
  #define INDEX_GEN_SLOT   3
#else
  #define AMP_GEN_SLOT     1        /* so that we can use setline instead */
  #define WAVET_GEN_SLOT   2
  #define INDEX_GEN_SLOT   3
#endif

FMINST::FMINST() : Instrument()
{
	modphs = carphs = 0.0;
}

int FMINST::init(float p[], int n_args)
{
	nsamps = rtsetoutput(p[0], p[1], this);

	sine = floc(WAVET_GEN_SLOT);
	if (sine == NULL)
		die("FMINST", "You need to store a waveform in function %d.",
					WAVET_GEN_SLOT);
	lensine = fsize(WAVET_GEN_SLOT);

	if (p[3] < 15.0)
		sicar = cpspch(p[3]) * lensine/SR;
	else
		sicar = p[3] * lensine/SR;
	
	if (p[4] < 15.0)
		simod = cpspch(p[4]) * lensine/SR;
	else
		simod = p[4] * lensine/SR;
	
	ampenv = floc(AMP_GEN_SLOT);
	if (ampenv) {
		int lenamp = fsize(AMP_GEN_SLOT);
		tableset(p[1], lenamp, amptabs);
	}
	else
		advise("FMINST", "Setting phrase curve to all 1's.");
	
	indexenv = floc(INDEX_GEN_SLOT);
	if (indexenv == NULL)
		die("FMINST", "You haven't made the index guide function (table %d).",
					INDEX_GEN_SLOT);
	lenind = fsize(INDEX_GEN_SLOT);
	tableset(p[1], lenind, indtabs);
	
	indbase = p[5];
	indbase *= simod;
	diff = (p[6] * simod) - indbase;
	
	amp = p[2];
	spread = p[7];
	skip = (int)(SR/(float)resetval);

	return(nsamps);
}

int FMINST::run()
{
	int i;
	float out[2];
	float aamp,val;
	int branch;
	float tfreq,tamp;

	Instrument::run();

	aamp = amp;             /* in case ampenv == NULL */

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (ampenv)
				aamp = table(cursamp, ampenv, amptabs) * amp;
			index = diff * tablei(cursamp,indexenv,indtabs) + indbase;
#ifdef RTUPDATE
			if (tags_on) {
				tfreq = rtupdate(this->mytag, 3);
				if (tfreq != NOPUPDATE)
					sicar = tfreq * (float)lensine/SR;
				tfreq = rtupdate(this->mytag, 4);
				if (tfreq != NOPUPDATE)
					simod = tfreq * (float)lensine/SR;
				tamp = rtupdate(this->mytag, 6);
				if (tamp != NOPUPDATE)
					diff = (tamp*simod) - indbase;
				tamp = rtupdate(this->mytag, 2);
				if (tamp != NOPUPDATE)
					amp = tamp;
			}
#endif
			branch = skip;
		}

		val = oscil(index,simod,sine,lensine,&modphs);
		out[0] = osciln(aamp,sicar+val,sine,lensine,&carphs);

		if (outputchans == 2) { /* split stereo files between the channels */
			out[1] = (1.0 - spread) * out[0];
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
		}
	return i;
}



Instrument*
makeFMINST()
{
	FMINST *inst;

	inst = new FMINST();
	inst->set_bus_config("FMINST");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("FMINST",makeFMINST);
}

