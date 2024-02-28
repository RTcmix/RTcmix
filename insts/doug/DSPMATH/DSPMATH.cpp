/* DSPMATH - DSP instrument which processes signal using one of a set of math functions

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = math function name (string)
   p5 = function-dependent

   p3 (amp) and p5 can receive updates from a table or real-time
   control source.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "DSPMATH.h"
#include <rt.h>
#include <rtdefs.h>
#include <math.h>
#include <string.h>

struct MathFun {
    const char *name;
    mathfunctionpointer function;
};

static double do_tanh(double in) { return tanh(in); }

static const struct MathFun math_functions[] = {
        { "tanh", &do_tanh },
        { 0, 0 }
};

static mathfunctionpointer findMathFunction(const char *name)
{
    const MathFun *funptr = &math_functions[0];
    while (funptr->name != NULL) {
        if (strcmp(name, funptr->name) == 0) {
            return funptr->function;
        }
        funptr++;
    }
    return NULL;
}

DSPMATH::DSPMATH()
	: _in(NULL), _branch(0), _drive(1/32768.0), _mathfun(NULL)
{
}

DSPMATH::~DSPMATH()
{
	delete [] _in;
}

int DSPMATH::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];

    _amp = p[3];

    _mathfun = findMathFunction(DOUBLE_TO_STRING(p[4]));
    if (_mathfun == NULL) {
        return die("DSPMATH", "Unknown math function");
    }

    // In order to use these math functions, the input needs to be scaled
    // to +-1.0, so we combine that in when we store this value.

    _drive = p[5] / 32768.0;

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("DSPMATH", "Use mono or stereo output only.");

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	return nSamps();
}


// Allocate the input buffer.  For non-interactive (script-driven) sessions,
// the constructor and init() for every instrument in the script are called
// before any of them runs.  By contrast, configure() is called right before
// the instrument begins playing.  If we were to allocate memory at init
// time, then all notes in the score would allocate memory then, resulting
// in a potentially excessive memory footprint.

int DSPMATH::configure()
{
    int status = 0;
    try {
        _in = new float[RTBUFSAMPS * inputChannels()];
    }
    catch(...) {
        status = -1;
    }
	return status;	// IMPORTANT: Return 0 on success, and -1 on failure.
}


// Called at the control rate to update parameters like amplitude, pan, etc.

void DSPMATH::doupdate()
{
	// The Instrument base class update() function fills the <p> array with
	// the current values of all pfields.  There is a way to limit the values
	// updated to certain pfields.  For more about this, read
	// src/rtcmix/Instrument.h.

	double p[6];
	update(p, 6);

    _amp = p[3];
    _drive = p[5] / 32768.0;      // see note in init(), above
}

int DSPMATH::run()
{
	const int samps = framesToRun() * inputChannels();

	rtgetin(_in, this, samps);

	// Each loop iteration processes 1 sample frame. */

	for (int i = 0; i < samps; i += inputChannels()) {

		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[4];

		for (int chan = 0; chan < inputChannels(); ++chan) {
			float insig = _in[i + chan];
			out[chan] = _mathfun(insig * _drive) * _amp;    // note gain applied after function
		}

		rtaddout(out);

		increment();
	}

	return framesToRun();
}


// The scheduler calls this to create an instance of this instrument
// and to set up the bus-routing fields in the base Instrument class.
// This happens for every "note" in a score.

Instrument *makeDSPMATH()
{
	DSPMATH *inst = new DSPMATH();
	inst->set_bus_config("DSPMATH");

	return inst;
}

#ifndef EMBEDDED

// The rtprofile introduces this instrument to the RTcmix core, and
// associates a script function name (in quotes below) with the instrument.
// This is the name the instrument goes by in a script.

void rtprofile()
{
	RT_INTRO("DSPMATH", makeDSPMATH);
}

#endif  // EMBEDDED

