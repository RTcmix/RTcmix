#include <iostream.h>
#include <stdio.h>
#include <mixerr.h>
#include <Instrument.h>
#include "WAVETABLE.h"
#include <rt.h>
#include <rtdefs.h>
#include <notetags.h>

extern "C" {
	#include <ugens.h>
	extern int resetval;
	}

WAVETABLE::WAVETABLE() : Instrument()
{
}

WAVETABLE::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = amplitude; p3 = frequency; p4 = stereo spread;
// real-time control enabled for p3 and p4

	nsamps = rtsetoutput(p[0], p[1], this);

	wavetable = floc(1);
	if (wavetable == NULL) {
		fprintf(stderr, "You need to store a waveform in function 1.\n");
		exit(1);
	}
	len = fsize(1);

	if (p[3] < 15.0) p[3] = cpspch(p[3]);
	si = p[3] * (float)len/SR;
	phase = 0.0;
	amp = p[2];
	
	amptable = floc(2);
	if (amptable) {
		alen = fsize(2);
		tableset(p[1], alen, tabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");
	
	spread = p[4];
	skip = (int)(SR/(float)resetval);

	return(nsamps);
}

int WAVETABLE::run()
{
	int i;
	float out[2];
	float aamp,tfreq,tamp;
	int branch;
	
	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, tabs) * amp;
			else
				aamp = amp;
			if (tags_on) {
				tfreq = rtupdate(this->mytag, 3);
				if (tfreq != NOPUPDATE)
					si = tfreq * (float)len/SR;
				tamp = rtupdate(this->mytag, 4);
				if (tamp != NOPUPDATE)
					amp = tamp;
				}
			branch = skip;
			}
		out[0] = oscili(aamp, si, wavetable, len, &phase);

		if (NCHANS == 2) { /* split stereo files between the channels */
		out[1] = (1.0 - spread) * out[0];
		out[0] *= spread;
		}
	  
		rtaddout(out);
		cursamp++;
		}
	return i;
}

Instrument*
makeWAVETABLE()
{
	WAVETABLE *inst;
	inst = new WAVETABLE();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("WAVETABLE",makeWAVETABLE);
}

