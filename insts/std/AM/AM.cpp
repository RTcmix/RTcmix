/* AM - amplitude modulation of input

   p0 = output start time
   p1 = input start time
   p2 = duration
   p3 = amplitude multiplier
   p4 = modulation oscillator frequency (or 0 to use function table)
   p5 = input channel [optional, default is 0]
   p6 = percent to left channel [optional, default is .5]

   Here are the function table assignments:

      1: amplitude curve (setline)
      2: AM modulator waveform (e.g., gen 9 or 10)
      3: AM modulator frequency curve (optional; set p4 to 0 to use table)

   Note that you get either amplitude modulation or ring modulation,
   depending on whether the modulator waveform is unipolar or bipolar
   (unipolar = amp. mod., bipolar = ring mod.).

   To make a unipolar sine wave, you have to add a DC component 90 degrees
   out of phase.  For example, the following makegen creates a sine wave
   that oscillates between 0 and 1:

      makegen(2, 9, 1000, 0,.5,90, 1,.5,0)


   Author unknown (probably Brad Garton).
   Modulator frequency table and xtra comments added by John Gibson, 1/12/02.
*/
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "AM.h"
#include <rt.h>
#include <rtdefs.h>


AM::AM() : Instrument()
{
	in = NULL;
	freqtable = NULL;
}

AM::~AM()
{
	delete [] in;
}


int AM::init(float p[], int n_args)
{
	int rvin;

	rvin = rtsetinput(p[1], this);
	if (rvin == -1) { // no input
		return(DONT_SCHEDULE);
	}
	nsamps = rtsetoutput(p[0], p[2], this);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(p[2], amplen, amptabs);
	}
	else
		advise("AM", "Setting phrase curve to all 1's.");

	amtable = floc(2);
	if (amtable == NULL) {
		die("AM", "You need a function table 2 containing mod. waveform.");
		return(DONT_SCHEDULE);
	}
	lenam = fsize(2);
	npoints = (float)lenam / SR;
	si = p[4] * npoints;

	if (si == 0.0) {
		freqtable = floc(3);
		if (freqtable) {
			int len = fsize(3);
      	tableset(getdur(), len, freqtabs);
		}
		else {
			die("AM", "Function table 3 must contain mod. freq. curve if p4=0.");
			return(DONT_SCHEDULE);
		}
	}

	amp = p[3];
	skip = (int)(SR/(float)resetval);      // how often to update amp curve
	phase = 0.0;

	inchan = (int)p[5];
	if ((inchan+1) > inputchans) {
		die("AM", "You asked for channel %d of a %d-channel file.",
																		inchan, inputchans);
		return(DONT_SCHEDULE);
	}

	spread = p[6];

	return(nsamps);
}

int AM::run()
{
	int i,rsamps;
	float out[2];
	float aamp;
	int branch;

	if (in == NULL)        /* first time, so allocate it */
		in = new float [RTBUFSAMPS * inputchans];

	rsamps = chunksamps*inputchans;

	rtgetin(in, this, rsamps);

	aamp = amp;            /* in case amptable == NULL */

	branch = 0;
	for (i = 0; i < rsamps; i += inputchans)  {
		if (--branch < 0) {
			if (amptable)
				aamp = tablei(cursamp, amptable, amptabs) * amp;
			if (freqtable)
				si = tablei(cursamp, freqtable, freqtabs) * npoints;
			branch = skip;
		}

		out[0] = in[i+inchan] * oscili(aamp, si, amtable, lenam, &phase);

		if (outputchans == 2) {
			out[1] = out[0] * (1.0 - spread);
			out[0] *= spread;
		}

		rtaddout(out);
		cursamp++;
	}
	return(i);
}



Instrument*
makeAM()
{
	AM *inst;

	inst = new AM();
	inst->set_bus_config("AM");

	return inst;
}

void
rtprofile()
{
	RT_INTRO("AM",makeAM);
}

