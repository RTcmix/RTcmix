#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "STEREO.h"
#include <rt.h>
#include <rtdefs.h>

/* If an input file has 2 chans, but there's only one mix matrix pfield,
   then the 2nd chan will be panned hard right (since an empty pfield has
   an implicit value of 0).  But I expect a channel with no matrix assignment
   to be disabled.  Uncommenting the define below gives an input chan with
   no explicit pfield a value of -1 in the mix matrix, which disables input
   from that channel.  This is here because I was bitten by this too many
   times.   JGG, 11/10/01
*/
#define DISABLE_UNASSIGNED_INPUT_CHANS

STEREO::STEREO() : Instrument()
{
	in = NULL;
	branch = 0;
}

STEREO::~STEREO()
{
	delete [] in;
}


#define MATRIX_PFIELD_OFFSET 4

int STEREO::init(float p[], int n_args)
{
// p0 = outsk; p1 = insk; p2 = dur (-endtime); p3 = amp; p4-n = channel mix matrix
// we're stashing the setline info in gen table 1

	int i, rvin;

	if (outputchans != 2) {
		die("STEREO", "Output must be stereo.");
		return(DONT_SCHEDULE);
	}

	if (n_args <= MATRIX_PFIELD_OFFSET) {
		die("STEREO", "You need at least one channel assignment.");
		return(DONT_SCHEDULE);
	}

	if (p[2] < 0.0) p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}

	amp = p[3];

	for (i = 0; i < inputchans; i++) {
		outspread[i] = p[i + MATRIX_PFIELD_OFFSET];
		}

#ifdef DISABLE_UNASSIGNED_INPUT_CHANS
	/* disable input chans that don't have explicit matrix assignments */
	i = n_args - MATRIX_PFIELD_OFFSET;
	for ( ; i < inputchans; i++)
		outspread[i] = -1.0;
#endif

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, tabs);
	}
	else
		advise("STEREO", "Setting phrase curve to all 1's.");
	aamp = amp;        /* in case amptable == NULL */

	skip = (int)(SR/(float)resetval);       // how often to update amp curve

	return(nsamps);
}

int STEREO::run()
{
	int i,j,rsamps;
	float out[2];

	if (in == NULL)    /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = table(cursamp, amptable, tabs) * amp;
			branch = skip;
			}

		out[0] = out[1] = 0.0;
		for (j = 0; j < inputchans; j++) {
			if (outspread[j] >= 0.0) {
				out[0] += in[i+j] * outspread[j] * aamp;
				out[1] += in[i+j] * (1.0 - outspread[j]) * aamp;
				}
			}

		rtaddout(out);
		cursamp++;
		}
	return i;
}



Instrument*
makeSTEREO()
{
	STEREO *inst;

	inst = new STEREO();
	inst->set_bus_config("STEREO");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("STEREO",makeSTEREO);
}
