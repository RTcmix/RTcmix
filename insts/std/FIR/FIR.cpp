#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <mixerr.h>
#include <Instrument.h>
#include "FIR.h"
#include <rt.h>
#include <rtdefs.h>


extern "C" {
	#include <ugens.h>
	extern int resetval;
}

FIR::FIR() : Instrument()
{
	// future setup here?
}

FIR::~FIR()
{
	delete [] pastsamps;
	delete [] coefs;
}


int FIR::init(float p[], short n_args)
{
/* fir: simple fir filter instrument
*
*  p0 = outsk
*  p1 = insk
*  p2 = dur
*  p3 = amp
*  p4 = total number of coefficients
*  p5...  the coefficients (up to 99 fir coefficients)
*
*  (no amplitude control, does channel 0 of both input and output only)
*
*/

	int i;

	rtsetinput(p[1], this);
	nsamps = rtsetoutput(p[0], p[2], this);

	ncoefs = (int)p[4];
	for (i = 0; i < ncoefs; i++) {
		coefs[i] = p[i+6]; 
		pastsamps[i] = 0.0;
	}

	amp = p[3];

	return(nsamps);
}

int FIR::run()
{
	int i,j,rsamps;
	float in[2*MAXBUF],out[2];

	rsamps = chunksamps*inputchans;
	rtgetin(in, this, rsamps);

	for (i = 0; i < chunksamps; i++) {
		out[0] = 0.0;
		pastsamps[0] = in[i*inputchans];

		for (j = 0; j < ncoefs; j++) 
			out[0] += (pastsamps[j] * coefs[j]);
		for (j = ncoefs-1; j >= 1; j--)
			pastsamps[j] = pastsamps[j-1];

		out[0] *= amp;
		rtaddout(out);
		cursamp++;
		}
	return(i);
}



Instrument*
makeFIR()
{
	FIR *inst;

	inst = new FIR();
	return inst;
}

void
rtprofile()
{
	RT_INTRO("FIR",makeFIR);
}

