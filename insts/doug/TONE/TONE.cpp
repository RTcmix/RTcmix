/* TONE - 1st order lowpass filter instrument

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = cutoff frequency

   p3 (amp) and p4 (cf) can receive updates from a table or real-time
   control source.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "TONE.h"
#include <rt.h>
#include <rtdefs.h>
#include <math.h>

/* -------------------------------------------------------------- toneset ---
 * */
/* toneset calculates the coefficients for tone.
   cutoff is -3db point in cps.  flag will reset history if set to 1.
*/
void
toneset(float SR, double cutoff, int flag, double *data)
{
   double x = 2.0 - cos(cutoff * PI2 / SR);	 /* feedback coeff. */
   data[1] = x - sqrt(x * x - 1.0);
   data[0] = 1.0 - data[1];					 /* gain coeff. */
   if (cutoff < 0.0)
	  data[1] *= -1.0;						  /* inverse function */
   if (flag)
	  data[2] = 0.0;
}

TONE::TONE()
	: _in(NULL), _branch(0)
{
}

TONE::~TONE()
{
	delete [] _in;
}

int TONE::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()

	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("TONE", "Use mono or stereo output only.");

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	for (int chan = 0; chan < inputChannels(); ++chan) {
		toneset(SR, SR/2.0, true, &toneData[chan][0]);
	}

	return nSamps();
}


// Allocate the input buffer.  For non-interactive (script-driven) sessions,
// the constructor and init() for every instrument in the script are called
// before any of them runs.  By contrast, configure() is called right before
// the instrument begins playing.  If we were to allocate memory at init
// time, then all notes in the score would allocate memory then, resulting
// in a potentially excessive memory footprint.

int TONE::configure()
{
	// RTBUFSAMPS is the maximum number of sample frames processed for each
	// call to run() below.

	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;	// IMPORTANT: Return 0 on success, and -1 on failure.
}


// Called at the control rate to update parameters like amplitude, pan, etc.

void TONE::doupdate()
{
	// The Instrument base class update() function fills the <p> array with
	// the current values of all pfields.  There is a way to limit the values
	// updated to certain pfields.  For more about this, read
	// src/rtcmix/Instrument.h.

	double p[5];
	update(p, 5);

	_amp = p[3];
	_cf = p[4];

	for (int chan = 0; chan < inputChannels(); ++chan) {
		toneset(SR, _cf, false, &toneData[chan][0]);
	}
}

inline double
tone(double sig, double *data)
{
	data[2] = data[0] * sig + data[1] * data[2];
	return data[2];
}

int TONE::run()
{
	const int samps = framesToRun() * inputChannels();

	rtgetin(_in, this, samps);

	// Each loop iteration processes 1 sample frame. */

	for (int i = 0; i < samps; i += inputChannels()) {

		// This block updates certain parameters at the control rate -- the
		// rate set by the user with the control_rate() or reset() script
		// functions.  The Instrument base class holds this value as a number
		// of sample frames to skip between updates.  Get this value using
		// getSkip() to reset the <_branch> counter.

		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[4];

		for (int chan = 0; chan < inputChannels(); ++chan) {

			// Grab the current input sample, scaled by the amplitude multiplier.

			float insig = _in[i + chan] * _amp;

			out[chan] = tone(insig, &toneData[chan][0]);
		}

		rtaddout(out);

		increment();
	}

	return framesToRun();
}


// The scheduler calls this to create an instance of this instrument
// and to set up the bus-routing fields in the base Instrument class.
// This happens for every "note" in a score.

Instrument *makeTONE()
{
	TONE *inst = new TONE();
	inst->set_bus_config("TONE");

	return inst;
}


// The rtprofile introduces this instrument to the RTcmix core, and
// associates a script function name (in quotes below) with the instrument.
// This is the name the instrument goes by in a script.

void rtprofile()
{
	RT_INTRO("TONE", makeTONE);
}


