/* Mix any number of inputs to stereo outputs with global amplitude control
   and individual pans.

      p0 = output start time
      p1 = input start time
      p2 = duration (-endtime)
      p3 = amplitude multiplier
      p4-n = channel mix maxtrix (see below)

   p3 (amplitude) can receive dynamic updates from a table or real-time
   control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p3 amplitude multiplier, even if the latter is dynamic.

   The mix matrix works like this.  For every input channel, the corresponding
   number in the matrix gives the output stereo pan for that channel, in
   percent-to-left form (0 is right; 1 is left).  p4 corresponds to input
   channel 0, p5 corresponds to input channel 1, etc.  If the value of one
   of these pfields is negative, then the corresponding input channel will 
   not be played.  Note that you cannot send a channel to more than one
   output pan location.

	Each mix matrix pfield (pctleft) can receive dynamic updates.

                                             rev. for v4.0 by JGG, 7/9/04
*/
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include "STEREO.h"
#include <rt.h>
#include <rtdefs.h>

STEREO::STEREO() : Instrument()
{
	in = NULL;
	branch = 0;
	warn_invalid = true;
}

STEREO::~STEREO()
{
	delete [] in;
}


#define MATRIX_PFIELD_OFFSET 4

int STEREO::init(double p[], int n_args)
{
	nargs = n_args;

	if (outputChannels() != 2)
		return die("STEREO", "Output must be stereo.");

	if (n_args <= MATRIX_PFIELD_OFFSET)
		return die("STEREO", "You need at least one channel assignment.");

	if (p[2] < 0.0)
		p[2] = -p[2] - p[1];

	nsamps = rtsetoutput(p[0], p[2], this);
	if (rtsetinput(p[1], this) == -1)
		return DONT_SCHEDULE;	// no input

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, p[2], amplen, tabs);
	}
	amp = p[3];

	outslots = n_args - MATRIX_PFIELD_OFFSET;

	skip = (int)(SR / (float) resetval);

	return nsamps;
}


int STEREO::configure()
{
	in = new float [RTBUFSAMPS * inputChannels()];
	return in ? 0 : -1;
}


// Fill the mix matrix.  An input chan with no explicit pfield gets a
// value of -1 in the matrix, which disables input from that channel.

void STEREO::updatePans(double p[])
{
	for (int i = 0; i < inputChannels(); i++) {
		double val = -1.0;
		if (i < outslots)
			val = p[i + MATRIX_PFIELD_OFFSET];
		if (val > 1.0) {
			if (warn_invalid) {
				warn("STEREO", "One or more pan values were greater than 1.");
				warn_invalid = false;
			}
			val = 1.0;
		}
		outspread[i] = val;
	}
}

int STEREO::run()
{
	int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inputChannels())  {
		if (--branch <= 0) {
			double p[nargs];
			update(p, nargs);
			amp = p[3];
			if (amptable)
				amp *= tablei(currentFrame(), amptable, tabs);
			updatePans(p);
			branch = skip;
		}

		float out[2];
		out[0] = out[1] = 0.0;
		for (int j = 0; j < inputChannels(); j++) {
			if (outspread[j] >= 0.0) {
				out[0] += in[i+j] * outspread[j] * amp;
				out[1] += in[i+j] * (1.0 - outspread[j]) * amp;
			}
		}

		rtaddout(out);
		increment();
	}
	return framesToRun();
}


Instrument *makeSTEREO()
{
	STEREO *inst;

	inst = new STEREO();
	inst->set_bus_config("STEREO");

	return inst;
}


void
rtprofile()
{
	RT_INTRO("STEREO", makeSTEREO);
}
