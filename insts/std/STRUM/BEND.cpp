#include <iostream.h>
#include "../../sys/mixerr.h"
#include "../../rtstuff/Instrument.h"
#include "BEND.h"
#include "../../rtstuff/rt.h"
#include "../../rtstuff/rtdefs.h"

extern strumq *curstrumq[6];

extern "C" {
	#include "../../H/ugens.h"
	void sset(float, float, float, strumq*);
	float strum(float, strumq*);
}

BEND::BEND() : Instrument()
{
	// future setup here?
}

int BEND::init(float p[], short n_args)
{
// p0 = start; p1 = dur; p2 = pitch0 (oct.pc); p3 = pitch1 (oct.pc);
// p4 = gliss function; p5 = fundamental decay time; p6 = nyquist decay time;
// p7 = update every nsamples; p8 = stereo spread [optional]

	int leng;

	nsamps = rtsetoutput(p[0], p[1], this);

	strumq1 = curstrumq[0];
	freq0 = cpspch(p[2]);
	freq1 = cpspch(p[3]);
	diff = freq1 - freq0;

	tf0 = p[5];
	tfN = p[6];
	sset(freq0, tf0, tfN, strumq1);

	glissf = floc((int)p[4]);
	leng = fsize((int)p[4]);
	tableset(p[1],leng,tags);

	resetval = p[7];
	if (resetval == 0) resetval = 100;
	spread = p[8];

	return(nsamps);
}

int BEND::run()
{
	int i;
	float freq;
	float out[2];
	int branch;

	branch = 0;
	for (i = 0; i < chunksamps; i++) {
		if (--branch < 0) {
			freq = diff * tablei(cursamp, glissf, tags) + freq0;
			sset(freq, tf0, tfN, strumq1);
			branch = resetval;
			}

		out[0] = strum(0.,strumq1);

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
makeBEND()
{
	BEND *inst;

	inst = new BEND();
	return inst;
}
