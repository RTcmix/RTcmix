/* MYSYNTH - sample code for a very basic synthesis instrument

   All it does is write noise into the output buffer, with optional
   panning.  Shows how to implement real-time control of parameters.
   Please send me suggestions for comment clarification.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = pan (in percent-to-left format) [optional, default is .5]

   p2 (amp) and p3 (pan) can receive updates from a table or real-time
   control source.

   John Gibson <johgibso at indiana dot edu>, 5/17/00; rev for v4, 6/14/05.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "MYSYNTH.h"			  // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>


// Construct an instance of this instrument and initialize some variables.
// Using an underbar as the first character of a data member is a nice
// convention to follow, but it's not necessary, of course.  It helps you
// to see at a glance whether you're looking at a local variable or a
// data member.

MYSYNTH::MYSYNTH()
	: _branch(0)
{
}


// Destruct an instance of this instrument, freeing any memory you've allocated.

MYSYNTH::~MYSYNTH()
{
}


// Called by the scheduler to initialize the instrument. Things done here:
//   - read, store and check pfields
//   - set output file (or bus) pointer
//   - init instrument-specific things
// If there's an error here (like invalid pfields), call and return die() to 
// report the error.  If you just want to warn the user and keep going,
// call warn() or rterror() with a message.

int MYSYNTH::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()

	// Store pfields in variables, to allow for easy pfield renumbering.
	// You should retain the RTcmix numbering convention for the first
	// 4 pfields: outskip, inskip, dur, amp; or, for instruments that 
	// take no input: outskip, dur, amp.

	const float outskip = p[0];
	const float dur = p[1];

	// Tell scheduler when to start this inst.  If rtsetoutput returns -1 to
	// indicate an error, then return DONT_SCHEDULE.

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	// Test whether the requested number of output channels is right for your
	// instrument.  The die function reports the error; the system decides
	// whether this should exit the program or keep going.

	if (outputChannels() > 2)
		return die("MYSYNTH", "Use mono or stereo output only.");

	// Return the number of sample frames that we'll write to output, which
	// the base class has already computed in response to our rtsetoutput call
	// above.  nSamps() equals the duration passed to rtsetoutput multiplied
	// by the sampling rate and then rounded to the nearest integer.

	return nSamps();
}


// For non-interactive (script-driven) sessions, the constructor and init()
// for every instrument in the script are called before any of them runs.
// By contrast, configure() is called right before the instrument begins
// playing.  If we were to allocate memory at init time, then all notes in
// the score would allocate memory then, resulting in a potentially excessive
// memory footprint.  So this is the place to allocate any substantial amounts
// of memory you might be using.

int MYSYNTH::configure()
{
	return 0;	// IMPORTANT: Return 0 on success, and -1 on failure.
}


// Called at the control rate to update parameters like amplitude, pan, etc.

void MYSYNTH::doupdate()
{
	// The Instrument base class update() function fills the <p> array with
	// the current values of all pfields.  There is a way to limit the values
	// updated to certain pfields.  For more about this, read
	// src/rtcmix/Instrument.h.

	double p[4];
	update(p, 4);

	_amp = p[2];

	// Here's how to handle an optional pfield.
	_pan = (_nargs > 3) ? p[3] : 0.5;           // default is .5
}


// Called by the scheduler for every time slice in which this instrument
// should run.  This is where the real work of the instrument is done.

int MYSYNTH::run()
{
	// framesToRun() gives the number of sample frames -- 1 sample for each
	// channel -- that we have to write during this scheduler time slice.
	// Each loop iteration outputs one sample frame.

	for (int i = 0; i < framesToRun(); i++) {

		// This block updates certain parameters at the control rate -- the
		// rate set by the user with the control_rate() or reset() script
		// functions.  The Instrument base class holds this value as a number
		// of sample frames to skip between updates.  Get this value using
		// getSkip() to reset the <_branch> counter.

		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[2];		// Space for only 2 output chans!

		// Write a random number, scaled by the amplitude multiplier, into
		// the output array.  rrand() is a function provided by RTcmix libgen
		// (see RTcmix/genlib).

		out[0] = rrand() * _amp;

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

Instrument *makeMYSYNTH()
{
	MYSYNTH *inst = new MYSYNTH();
	inst->set_bus_config("MYSYNTH");

	return inst;
}


// The rtprofile introduces this instrument to the RTcmix core, and
// associates a script function name (in quotes below) with the instrument.
// This is the name the instrument goes by in a script.

void rtprofile()
{
	RT_INTRO("MYSYNTH", makeMYSYNTH);
}


