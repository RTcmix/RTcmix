#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "SCULPT.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"

extern "C" {
	#include "../../H/ugens.h"
}

SCULPT::SCULPT() : Instrument()
{
	// future setup here?
}

int SCULPT::init(float p[], short n_args)
{
// p0 = start; p1 = point dur; p2 = overall amplitude; p3 = number of points
// p4 = stereo spread [optional];
// function slot 1 is waveform, slot 2 is overall amp envelope
// function slot 3 is frequency points, slot 4 is amplitude points

	float tdur;

	nsamps = rtsetoutput(p[0], p[1]*p[3], this);
	tdur = p[1] * p[3];
	pdur = p[1] * SR;

	wave = floc(1);
	len = fsize(1);

	amptable = floc(2);
	tableset(tdur, fsize(2), amptabs);
	freqtable = floc(3);
	pamptable = floc(4);

	amp = p[2];
	phase = 0.0;
	pcount = 0;
	index = 0;

	spread = p[4];

	return(nsamps);
}

int SCULPT::run()
{
	int i;
	float out[2];
	float si;
	float overamp, aamp;

	for (i = 0; i < chunksamps; i++) {
		if (--pcount < 0) {
			si = freqtable[index] * (float)len/SR;
			overamp = table(cursamp, amptable, amptabs) * amp;
			aamp = ampdb(60.0 + pamptable[index]) * overamp;
			index++;
			pcount = pdur;
			}
		out[0] = oscil(aamp, si, wave, len, &phase);

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
makeSCULPT()
{
	SCULPT *inst;

	inst = new SCULPT();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("SCULPT",makeSCULPT);
}

