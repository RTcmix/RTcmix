#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "INPUTSIG.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	#include <ugens.h>
	extern int resetval;
	extern float rsnetc[64][5],amp[64];
	extern int nresons;
}

INPUTSIG::INPUTSIG() : Instrument()
{
	// future setup here?
}

int INPUTSIG::init(float p[], short n_args)
{
// p0 = output skip; p1 = input skip; p2 = duration
// p3 = amplitude multiplier; p4 = input channel (0 or 1)
// p5 = stereo spread (0-1) [optional]
// assumes function table 1 is the amplitude envelope

	int i;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2], this);

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(p[2], lenamp, amptabs);
	}
	else
		printf("Setting phrase curve to all 1's\n");

	for(i = 0; i < nresons; i++) {
		myrsnetc[i][0] = rsnetc[i][0];
		myrsnetc[i][1] = rsnetc[i][1];
		myrsnetc[i][2] = rsnetc[i][2];
		myrsnetc[i][3] = myrsnetc[i][4] = 0.0;
		myamp[i] = amp[i];
		}
	mynresons = nresons;

	oamp = p[3];
	inchan = (int)p[4];
	if ((inchan+1) > inputchans) {
	fprintf(stderr,"uh oh, you have asked for channel %d of a %d-channel file...\n",inchan,inputchans);
		exit(-1);
		}

	skip = (int)(SR/(float)resetval);
	spread = p[5];

	return(nsamps);
}

int INPUTSIG::run()
{
	int i,j,rsamps;
	float in[2*MAXBUF],out[2];
	float aamp,val;
	int branch;

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = oamp;           /* in case amparr == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amparr)
				aamp = tablei(cursamp, amparr, amptabs) * oamp;
			branch = skip;
			}

		out[0] = 0.0;
		for(j = 0; j < mynresons; j++) {
			val = reson(in[i+inchan],myrsnetc[j]);
			out[0] += val * myamp[j];
			}

		out[0] *= aamp;
		if (NCHANS == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
			}

		rtaddout(out);
		cursamp++;
		}
	return(i);
}



Instrument*
makeINPUTSIG()
{
	INPUTSIG *inst;

	inst = new INPUTSIG();
	return inst;
}
