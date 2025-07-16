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
#include <ugens.h>
#include "NOTE.h"          // declarations for this instrument class
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

// This treats MIDI velocity 0 as a note-off

void NOTE::doStart(FRAMETYPE frameOffset)
{
    long timestamp = getEventTimestamp(frameOffset);
    int vel = (int)(0.5 + 127.0*_midiVel);
    if (vel > 0) {
        PRINT("NOTE: %p sending note on chan %d note %d vel %d with frame offset = %llu => timestamp %ld\n", this,
              _midiChannel, _midiNote, vel, frameOffset, timestamp);
        _outputPort->sendNoteOn(timestamp, (unsigned char) _midiChannel, (unsigned char) _midiNote,
                                (unsigned char) vel);
    }
    else {
        PRINT("NOTE: %p sending note off chan %d note %d (due to zero vel) with frame offset = %llu => timestamp %ld\n", this,
              _midiChannel, _midiNote, frameOffset, timestamp);
        _outputPort->sendNoteOff(timestamp, (unsigned char) _midiChannel, (unsigned char) _midiNote, 0);
    }
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void NOTE::doupdate(FRAMETYPE currentFrame)
{
}

void NOTE::doStop(FRAMETYPE frameOffset) {
    long timestamp = getEventTimestamp(frameOffset);
    if ((int) (0.5 + 127.0 * _midiVel) > 0) {
        PRINT("NOTE: %p sending note off chan %d note %d with frame offset = %llu => timestamp %ld\n", this, _midiChannel,
               _midiNote, frameOffset, timestamp);
        _outputPort->sendNoteOff(timestamp, (unsigned char) _midiChannel, (unsigned char) _midiNote, 0);
    }
    else {
        PRINT("NOTE: skipping note-off on stop because this was a zero-velocity event\n");
    }
}

Instrument *makeNOTE()
{
	NOTE *inst = new NOTE();
	inst->set_bus_config("NOTE");

	return inst;
}

