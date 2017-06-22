/* EXPDELAY - Feedback delay at exponentially changing delay times

   TODO: CHANGEME CHANGEME CHANGEME

   p0 = output start time
   p1 = input start time
	 p2 = amplitude multiplier
	 p3 = input channel
	 p4 = pan
   p5 = maximum decay time
	 p6 = number of repetitions
	 p7 = decay time
	 p8 = amplitude curve
   p9 = decay curve

   p2, p4, and p8 can receive updates from a table or real-time control source.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "EXPDELAY.h"          // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>

// Construct an instance of this instrument and initialize some variables.
// Using an underbar as the first character of a data member is a nice
// convention to follow, but it's not necessary, of course.  It helps you
// to see at a glance whether you're looking at a local variable or a
// data member.

EXPDELAY::EXPDELAY()
	: _in(NULL), _branch(0)
{
}


// Destruct an instance of this instrument, freeing memory for the input buffer,
// unit generator objects, etc.

EXPDELAY::~EXPDELAY()
{
	delete [] _in;
}


// Called by the scheduler to initialize the instrument. Things done here:
//   - read, store and check pfields
//   - set input and output file (or bus) pointers
//   - init instrument-specific things
// If there's an error here (like invalid pfields), call and return die() to
// report the error.  If you just want to warn the user and keep going,
// call warn() or rterror() with a message.

int EXPDELAY::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()
	printf("SR: %f", SR);
	// Store pfields in variables, to allow for easy pfield renumbering.
	// You should retain the RTcmix numbering convention for the first
	// 4 pfields: outskip, inskip, dur, amp; or, for instruments that
	// take no input: outskip, dur, amp.

	// no need to retrieve amp or pan here, because these will be set
	// before their first use inside of doupdate().
	const float outskip = p[0];
	const float inskip = p[1];
	const float amp = 1;//p[2];
	const int inchan = 0;//p[3];
	const float pan = 0.5;//p[4];
	const float max_delay_time = 10;//p[5];
	const int reps = 20;//p[6];
	const float delay_time = 10;//p[7];
	const float ampcurve = 1;//p[8];
	const float decaycurve = 1;//p[9];

	switch (n_args)
	{
		case 10:
			_decay_curve = p[9];
		case 9:
			_amp_curve = p[8];
		case 8:
			_delay_time = p[7];
		case 7:
			_delay_reps = (int)p[6];
		case 6:
			_max_delay_time = p[5];
		case 5:
			_pan = p[4];
		case 4:
			_inchan = (int)p[3];
		case 3:
			_amp = p[2];
		default:
		break;
	}

	_delay_buf = new float[(int)(SR*_max_delay_time)];

	// Tell scheduler when to start this inst.  If rtsetoutput returns -1 to
	// indicate an error, then return DONT_SCHEDULE.

	if (rtsetoutput(outskip, _delay_time, this) == -1)
		return DONT_SCHEDULE;

	// Test whether the requested number of output channels is right for your
	// instrument.  The die function reports the error; the system decides
	// whether this should exit the program or keep going.

	if (outputChannels() > 2)
		return die("EXPDELAY", "Use mono or stereo output only.");

	// Set file pointer on audio input.  If the input source is real-time or
	// an aux bus, then <inskip> must be zero.  The system will return an
	// an error in this case, which we must pass along.

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	// Make sure requested input channel number is valid for this input source.
	// inputChannels() gives the total number of input channels, initialized
	// in rtsetinput.

	if (_inchan >= inputChannels())
		return die("EXPDELAY", "You asked for channel %d of a %d-channel input.",
		                                             _inchan, inputChannels());

	// Return the number of sample frames that we'll write to output, which
	// the base class has already computed in response to our rtsetoutput call
	// above.  nSamps() equals the duration passed to rtsetoutput multiplied
	// by the sampling rate and then rounded to the nearest integer.

	return nSamps();
}


// Allocate the input buffer.  For non-interactive (script-driven) sessions,
// the constructor and init() for every instrument in the script are called
// before any of them runs.  By contrast, configure() is called right before
// the instrument begins playing.  If we were to allocate memory at init
// time, then all notes in the score would allocate memory then, resulting
// in a potentially excessive memory footprint.

int EXPDELAY::configure()
{
	// RTBUFSAMPS is the maximum number of sample frames processed for each
	// call to run() below.

	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;	// IMPORTANT: Return 0 on success, and -1 on failure.
}


// Called at the control rate to update parameters like amplitude, pan, etc.

void EXPDELAY::doupdate()
{
	// The Instrument base class update() function fills the <p> array with
	// the current values of all pfields.  There is a way to limit the values
	// updated to certain pfields.  For more about this, read
	// src/rtcmix/Instrument.h.

	double p[6];
	update(p, 6);

	_amp = p[3];

	// Here's how to handle an optional pfield.
	_pan = (_nargs > 5) ? p[5] : 0.5;           // default is .5
}


// Called by the scheduler for every time slice in which this instrument
// should run.  This is where the real work of the instrument is done.

int EXPDELAY::run()
{
	// framesToRun() gives the number of sample frames -- 1 sample for each
	// channel -- that we have to write during this scheduler time slice.

	const int samps = framesToRun() * inputChannels();

	// Read <samps> samples from the input file (or audio input device).

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

		// Grab the current input sample, scaled by the amplitude multiplier.

		float insig = _in[i + _inchan] * _amp;

		float out[2];		// Space for only 2 output chans!

		// Just copy it to the output array with no processing.

		out[0] = insig;

		// If we have stereo output, use the pan pfield.

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		// Write this sample frame to the output buffer.

		rtaddout(out);

		// Increment the count of sample frames this instrument has written.

		increment();
	}

	// Return the number of frames we processed.

	return framesToRun();
}


// The scheduler calls this to create an instance of this instrument
// and to set up the bus-routing fields in the base Instrument class.
// This happens for every "note" in a score.

Instrument *makeEXPDELAY()
{
	EXPDELAY *inst = new EXPDELAY();
	inst->set_bus_config("EXPDELAY");

	return inst;
}


// The rtprofile introduces this instrument to the RTcmix core, and
// associates a script function name (in quotes below) with the instrument.
// This is the name the instrument goes by in a script.

void rtprofile()
{
	RT_INTRO("EXPDELAY", makeEXPDELAY);
}
