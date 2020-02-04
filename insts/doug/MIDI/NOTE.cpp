/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
/* NOTE - instrument which generates a MIDI note-on/off

   p0 = output start time
   p1 = output duration
   p2 = MIDI channel (0-15)
   p3 = pitch (Octave point PC)
   p4 = MIDI velocity   (normalized, 0.0 to 1.0)
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "NOTE.h"          // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>
#include <RTMIDIOutput.h>

#define DEBUG 0

#if DEBUG
#define PRINT printf
#else
#define PRINT if (0) printf
#endif

NOTE::NOTE() : MIDIBase(), _midiNote(0), _midiVel(0)
{
}

NOTE::~NOTE()
{
}

int NOTE::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _midiNote = (int)(0.5 + midipch(p[3]));     // midipch returns a float, so make sure we round appropriately
    _midiVel = p[4];
    
    // Limit results rather than return an error.
    if (_midiNote < 0) {
        rtcmix_warn("NOTE", "Note number limited to 0");
        _midiNote = 0;
    }
    else if (_midiNote > 127) {
        rtcmix_warn("NOTE", "Note number limited to 127");
        _midiNote = 127;
    }
    if (_midiVel < 0.0) {
        rtcmix_warn("NOTE", "Velocity limited to 0");
        _midiVel = 0;
    }
    else if (_midiVel > 1.0) {
        rtcmix_warn("NOTE", "Velocity limited to 1.0");
        _midiVel = 1.0;
    }

//    PRINT("NOTE: %p chan %d note %d vel %f (normalized)\n", this, _midiChannel, _midiNote, _midiVel);
    return nSamps();
}

void NOTE::doStart(FRAMETYPE frameOffset)
{
    long timestamp = (1000.0 * frameOffset) / SR;
    int vel = (int)(0.5 + 127.0*_midiVel);
    PRINT("NOTE: %p sending note on chan %d note %d vel %d with offset %ld\n", this, _midiChannel, _midiNote, vel, timestamp);
    _outputPort->sendNoteOn(timestamp, _midiChannel, _midiNote, vel);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void NOTE::doupdate(FRAMETYPE currentFrame)
{
}

void NOTE::doStop(FRAMETYPE currentFrame)
{
    long timestamp = 1000.0 * (currentFrame - getRunStartFrame()) / SR;
    PRINT("NOTE: %p sending note off with offset %ld\n", this, timestamp);
    _outputPort->sendNoteOff(timestamp, _midiChannel, _midiNote, 0);
}

Instrument *makeNOTE()
{
	NOTE *inst = new NOTE();
	inst->set_bus_config("NOTE");

	return inst;
}

