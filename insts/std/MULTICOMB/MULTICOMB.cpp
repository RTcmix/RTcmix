#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "MULTICOMB.h"
#include <rt.h>
#include <rtdefs.h>


MULTICOMB::MULTICOMB() : Instrument()
{
	in = NULL;
	for (int n=0; n<NCEES; n++)
		carray[n] = NULL;	
}

MULTICOMB::~MULTICOMB()
{
	delete [] in;
	for (int n=0; n<NCEES; n++)
		delete [] carray[n];	
}

int MULTICOMB::init(float p[], int n_args)
{
// p0 = output skip; p1 = input skip; p2 = output duration
// p3 = amplitude multiplier; p4 = comb frequency range bottom
// p5 = comb frequency range top; p6 = reverb time
//  assumes function table 1 is the amplitude envelope

	int i,j,nmax,rvin;
	float cfreq;

	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
	nsamps = rtsetoutput(p[0], p[2], this);

	if (outputchans != 2) {
		die("MULTICOMB", "Sorry, output must be stereo.");
		return(DONT_SCHEDULE);
	}

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		advise("MULTICOMB", "Setting phrase curve to all 1's.");

	for (j = 0; j < NCEES; j++) {
		cfreq = (p[5] - p[4]) *  (rrand()+2.0)/2.0  + p[4];
		advise(NULL, "comb number %d: %f\n",j,cfreq);
		nmax = (int)(SR/(int)cfreq + 4);
		if ( (carray[j] = new float[nmax] )  == NULL)
			die("MULTICOMB", "Sorry, Charlie -- no space");
		for (i = 0; i < nmax; i++) carray[j][i] = 0.0;

		combset(1.0/cfreq,p[6],0,carray[j]);
		spread[j] = (float)j/(float)(NCEES-1);
		}

	amp = p[3];
	skip = (int)(SR/(float)resetval);    // how often to update amp curve

	return(nsamps);
}

int MULTICOMB::run()
{
	int i,j,rsamps;
	float out[2];
	float aamp,temp;
	int branch;

	if (in == NULL)        /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;          /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			branch = skip;
			}

		out[0] = out[1] = 0.0;
		for (j = 0; j < NCEES; j++) {
			temp = comb(in[i], carray[j]);
			out[0] += temp * spread[j]; 
			out[1] += temp * (1.0 - spread[j]);
			}

		out[0] *= aamp;
		out[1] *= aamp;

		rtaddout(out);
		cursamp++;
		}
	return(i);
}



Instrument*
makeMULTICOMB()
{
	MULTICOMB *inst;

	inst = new MULTICOMB();
	inst->set_bus_config("MULTICOMB");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("MULTICOMB",makeMULTICOMB);
}

