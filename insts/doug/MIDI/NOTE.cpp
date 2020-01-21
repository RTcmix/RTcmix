/* NOTE - instrument which generates a MIDI note-on/off

   p0 = output start time
   p1 = output duration
   p2 = amplitude multiplier
   p3 = MIDI channel
   p4 = pitch (Octave point PC)
   p5 = MIDI velocity
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "NOTE.h"          // declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>
#include <RTMIDIOutput.h>

NOTE::NOTE() : MIDIBase(), _midiNote(0), _midiVel(0)
{
}

NOTE::~NOTE()
{
}

// Called by the scheduler to initialize the instrument. Things done here:
//   - read, store and check pfields
//   - set input and output file (or bus) pointers
//   - init instrument-specific things
// If there's an error here (like invalid pfields), call and return die() to 
// report the error.  If you just want to warn the user and keep going,
// call warn() or rterror() with a message.

int NOTE::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _midiNote = (int)(0.5 + midipch(p[4]));     // midipch returns a float, so make sure we round appropriately
    _midiVel = (int)(127*p[5]);
    
    if (_midiNote < 0 || _midiNote > 127) {
        return die("NOTE", "Illegal pitch for this instrument");
    }
    if (_midiVel < 0 || _midiVel > 127) {
        return die("NOTE", "Illegal velocity");
    }

    printf("Chan %d, note %d, vel %d\n", _midiChannel, _midiNote, _midiVel);

    return nSamps();
}

void NOTE::doStart()
{
    _outputPort->sendNoteOn(0, _midiChannel, _midiNote, _midiVel);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void NOTE::doupdate(FRAMETYPE currentFrame)
{
	// The Instrument base class update() function fills the <p> array with
	// the current values of all pfields.  There is a way to limit the values
	// updated to certain pfields.  For more about this, read
	// src/rtcmix/Instrument.h.

	double p[5];
	update(p, 5);

	_amp = p[2];
}

void NOTE::doStop(FRAMETYPE currentFrame)
{
    long timestamp = 1000.0 * (currentFrame - getRunStartFrame()) / SR;
//    printf("NOTE: sending note off\n");
    _outputPort->sendNoteOff(timestamp, _midiChannel, _midiNote, 0);
}

Instrument *makeNOTE()
{
	NOTE *inst = new NOTE();
	inst->set_bus_config("NOTE");

	return inst;
}

