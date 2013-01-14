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
#include <PField.h>
#include <Option.h>	// fastUpdate
#include "STEREO.h"
#include <rt.h>
#include <rtdefs.h>


STEREO::STEREO() : Instrument()
{
	in = NULL;
	branch = 0;
	warnInvalid = true;
}


STEREO::~STEREO()
{
	delete [] in;
}


// In fastUpdate mode, we skip doupdate() entirely, instead updating only amp,
// and only from a table.  The table can be a makegen or a PField table.  PField
// tables must be "flattened" using copytable if they are compound (e.g. passed
// through a PField filter or multiplied by a constant).  We use p[ampindex] as
// an amp multiplier, unless using a PField table, in which case there is no amp
// multiplier -- the p[ampindex] value is the first table value.   -JGG

void STEREO::initamp(float dur, double p[], int ampindex, int ampgenslot)
{
	fastUpdate = Option::fastUpdate();
	if (fastUpdate) {
		// Prefer PField table, otherwise makegen
		int tablen = 0;
		amptable = (double *) getPFieldTable(ampindex, &tablen);
		if (amptable)
			ampmult = 1.0f;
		else {
			ampmult = p[ampindex];
			amptable = floc(ampgenslot);
			if (amptable)
				tablen = fsize(ampgenslot);
		}
		if (amptable)
			tableset(SR, dur, tablen, amptabs);
		else
			amp = ampmult;
	}
	else {
		// NB: ampmult never used, first amp set in doupdate
		amptable = floc(ampgenslot);
		if (amptable) {
			int tablen = fsize(ampgenslot);
			tableset(SR, dur, tablen, amptabs);
		}
	}
}


#define MATRIX_PFIELD_OFFSET 4

int STEREO::init(double p[], int n_args)
{
	nargs = n_args;
	outslots = n_args - MATRIX_PFIELD_OFFSET;
	const float outskip = p[0];
	const float inskip = p[1];
	float dur = p[2];
	if (dur < 0.0)
		dur = -dur - inskip;

	if (n_args <= MATRIX_PFIELD_OFFSET)
		return die("STEREO", "You need at least one channel assignment.");

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;	// no input

	if (outputChannels() != 2)
		return die("STEREO", "Output must be stereo.");

	initamp(dur, p, 3, 1);
	if (fastUpdate)
		updatePans(p);

	return nSamps();
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
	const int inchans = inputChannels();
	for (int i = 0; i < inchans; i++) {
		double val = -1.0;
		if (i < outslots)
			val = p[i + MATRIX_PFIELD_OFFSET];
		if (val > 1.0) {
			if (warnInvalid) {
				rtcmix_warn("STEREO", "One or more pan values were greater than 1.");
				warnInvalid = false;
			}
			val = 1.0;
		}
		outPan[i] = val;
	}
}


void STEREO::doupdate()
{
	double p[nargs];
	update(p, nargs);

	amp = p[3];
	if (amptable)
		amp *= tablei(currentFrame(), amptable, amptabs);

	updatePans(p);
}


int STEREO::run()
{
	const int inchans = inputChannels();
	const int samps = framesToRun() * inchans;

	rtgetin(in, this, samps);

	for (int i = 0; i < samps; i += inchans)  {
		if (--branch <= 0) {
			if (fastUpdate) {
				if (amptable)
					amp = ampmult * tablei(currentFrame(), amptable, amptabs);
			}
			else
				doupdate();
			branch = getSkip();
		}

		float out[2];
		out[0] = out[1] = 0.0;
		for (int j = 0; j < inchans; j++) {
			if (outPan[j] >= 0.0) {
				out[0] += in[i+j] * outPan[j] * amp;
				out[1] += in[i+j] * (1.0 - outPan[j]) * amp;
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

#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("STEREO", makeSTEREO);
}
#endif
