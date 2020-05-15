/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  PROGRAM.cpp
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = MIDI channel
 p3 = Patch number (integer)
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

int PROGRAM::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _patchNumber = (int)(p[3]);

    if (_patchNumber < 0) {
        rtcmix_warn("PROGRAM", "Patch number limited to 0");
        _patchNumber = 0;
    }
    else if (_patchNumber > 127) {
        rtcmix_warn("PROGRAM", "Patch number limited to 127");
        _patchNumber = 127;
    }
    return nSamps();
}

void PROGRAM::doStart(FRAMETYPE frameOffset)
{
//    printf("Sending MIDI program number %d\n", _patchNumber);
    long timestamp = (1000.0 * frameOffset) / SR;
    _outputPort->sendProgramChange(timestamp, _midiChannel, _patchNumber);
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
