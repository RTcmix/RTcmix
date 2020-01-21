//
//  PROGRAM.cpp
//  RTcmix Desktop
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = amplitude multiplier
 p3 = MIDI channel
 p4 = Patch number (integer)
*/

#include "PROGRAM.h"
#include "RTMIDIOutput.h"
#include <ugens.h>
#include <rt.h>
#include <stdio.h>
#include <string.h>

PROGRAM::PROGRAM() : MIDIBase(), _patchNumber(0)
{
}

PROGRAM::~PROGRAM()
{
}

// Called by the scheduler to initialize the instrument. Things done here:
//   - read, store and check pfields
//   - set input and output file (or bus) pointers
//   - init instrument-specific things
// If there's an error here (like invalid pfields), call and return die() to
// report the error.  If you just want to warn the user and keep going,
// call warn() or rterror() with a message.

int PROGRAM::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _patchNumber = (int)(p[4]);

    printf("Chan %d, patch %d\n", _midiChannel, _patchNumber);

    if (_patchNumber < 0 || _patchNumber > 127) {
        return die("PROGRAM", "Program number must be between 0 and 127");
    }
    return nSamps();
}

void PROGRAM::doStart()
{
    printf("Sending MIDI program number %d\n", _patchNumber);
    _outputPort->sendProgramChange(0, _midiChannel, _patchNumber);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void PROGRAM::doupdate(FRAMETYPE currentFrame)
{
}

Instrument *makePROGRAM()
{
    PROGRAM *inst = new PROGRAM();
    inst->set_bus_config("PROGRAM");
    
    return inst;
}
