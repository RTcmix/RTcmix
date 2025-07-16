/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
/* MIDIBase - base class for instruments which generate MIDI output

   p0 = output start time
   p1 = output duration
   p2 = MIDI channel
*/
#include <stdio.h>
#include <ugens.h>
#include "MIDIBase.h"          // declarations for this instrument class
#include <RTMIDIOutput.h>

#define DEBUG 0

#if DEBUG
#define PRINT printf
#else
#define PRINT if (0) printf
#endif

extern RTMIDIOutput *getMIDIOutput();

MIDIBase::MIDIBase()
    : _branch(0), _midiChannel(0), _nargs(0), _runStartFrame(0), _outputPort(NULL)
{
}

MIDIBase::~MIDIBase()
{
}

int MIDIBase::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()

    _outputPort = ::getMIDIOutput();
    if (_outputPort == NULL) {
        return die(name(), "You need to set up MIDI output before playing instruments");
    }

    const float outskip = p[0];
	const float dur = p[1];
    _midiChannel = (int)p[2];
    if (_midiChannel < 0 || _midiChannel > 15) {
        return die(name(), "Illegal MIDI channel");
    }

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

    return nSamps();
}

int MIDIBase::configure()
{
    return 0;
}

// Called by the scheduler for every time slice in which this instrument
// should run.  This is where the real work of the instrument is done.

int MIDIBase::run()
{
    _runStartFrame = currentFrame();
    // We only do work for the first frame.  We access the offset into the audio buffer via the base
    // class 'output_offset'
    int frameOffset = 0;
    if (_runStartFrame == 0) {
        frameOffset = this->output_offset;
        PRINT("MIDIBase::run(%p) calling doStart(frameOffset = %d)\n", this, frameOffset);
        doStart(frameOffset);
    }

    // Note:  We cannot cache currentFrame() because it updates with increment()
    
    const int frameCount = framesToRun();
    const int end = nSamps() - 1;
    
    // Because the MIDI instruments don't generate audio directly, we fake the
    // loop here so we can call doUpdate() as many times as is expected for this
    // render call.
    
    for (int fr = 0; fr < frameCount; ++fr) {
        int current = currentFrame();
        if (--_branch <= 0) {
            doupdate(frameOffset+fr);
            _branch = getSkip();
        }
        increment();
        if (current == end) {
            PRINT("MIDIBase::run(%p) calling doStop(frameOffset = %d)\n", this, frameOffset+fr);
            doStop(frameOffset+fr);
            break;
        }
    }

    // Return the number of frames we processed.
	return framesToRun();
}

long MIDIBase::getEventTimestamp(FRAMETYPE frameOffset)
{
    long timestamp = (1000.0 * (get_ichunkstart() + frameOffset)) / SR;
    return timestamp;
}
