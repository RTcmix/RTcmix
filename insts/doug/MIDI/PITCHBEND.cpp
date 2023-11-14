/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  PITCHBEND.cpp
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = MIDI channel (0-15)
 p3 = Bend value (-1.0 to 1.0, normalized)
*/

#include "PITCHBEND.h"
#include "RTMIDIOutput.h"
#include <ugens.h>
#include <rt.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 0

#if DEBUG
#define PRINT printf
#else
#define PRINT if (0) printf
#endif

PITCHBEND::PITCHBEND() : MIDIBase(), _bendValue(0.0)
{
}

PITCHBEND::~PITCHBEND()
{
}

int PITCHBEND::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _bendValue = p[3];

    if (_bendValue < -1.0) {
        rtcmix_warn("PITCHBEND", "Value limited to -1.0");
        _bendValue = -1.0;
    }
    else if (_bendValue > 1.0) {
        rtcmix_warn("PITCHBEND", "Value limited to 1.0");
        _bendValue = 1.0;
    }
    
    PRINT("Chan %d value %f (normalized)\n", _midiChannel, _bendValue);

    return nSamps();
}

void PITCHBEND::doStart(FRAMETYPE frameOffset)
{
    long timestamp = (1000.0 * frameOffset) / SR;
    unsigned value = 16383.5 + unsigned(0.5 + (_bendValue * 16383));
    PRINT("doStart sending pitch bend on chan %d: value %u at offset %ld\n", _midiChannel, value, timestamp);
    _outputPort->sendPitchBend(timestamp, (unsigned char)_midiChannel, value);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void PITCHBEND::doupdate(FRAMETYPE currentFrame)
{
    // The Instrument base class update() function fills the <p> array with
    // the current values of all pfields.  There is a way to limit the values
    // updated to certain pfields.  For more about this, read
    // src/rtcmix/Instrument.h.
    
    double p[4];
    update(p, 5, 1 << 3);
    
    float newValue = p[3];
    if (newValue < -1.0) {
        rtcmix_warn("PITCHBEND", "Controller value limited to -1.0");
        newValue = -1.0;
    }
    else if (newValue > 1.0) {
        rtcmix_warn("PITCHBEND", "Controller value limited to 1.0");
        newValue = 1.0;
    }
    if (newValue != _bendValue) {
        unsigned value = 16383.5 + unsigned(0.5 + (_bendValue * 16383));
        long timestamp = 1000.0 * (currentFrame - getRunStartFrame()) / SR;
        PRINT("doUpdate sending pitch bend value %u with frame time offset %ld ms\n", value, timestamp);
        _outputPort->sendPitchBend(timestamp, (unsigned char)_midiChannel, value);
        _bendValue = newValue;
    }
}

Instrument *makePITCHBEND()
{
    PITCHBEND *inst = new PITCHBEND();
    inst->set_bus_config("PITCHBEND");
    
    return inst;
}
