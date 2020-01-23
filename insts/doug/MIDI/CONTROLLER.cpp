/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  CONTROLLER.cpp
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = MIDI channel (0-15)
 p3 = Controller number (integer, 0-127)
 p4 = Controller value (normalize, 0.0 - 1.0)
*/

#include "CONTROLLER.h"
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

CONTROLLER::CONTROLLER() : MIDIBase(), _controllerNumber(0), _controllerValue(0.0)
{
}

CONTROLLER::~CONTROLLER()
{
}

// Called by the scheduler to initialize the instrument. Things done here:
//   - read, store and check pfields
//   - set input and output file (or bus) pointers
//   - init instrument-specific things
// If there's an error here (like invalid pfields), call and return die() to
// report the error.  If you just want to warn the user and keep going,
// call warn() or rterror() with a message.

int CONTROLLER::init(double p[], int n_args)
{
    if (MIDIBase::init(p, n_args) < 0) {
        return DONT_SCHEDULE;
    }
    _controllerNumber = (int)(p[3]);
    _controllerValue = p[4];

    if (_controllerNumber < 0) {
        rtcmix_warn("CONTROLLER", "Controller number limited to 0");
        _controllerNumber = 0;
    }
    else if (_controllerNumber > 127) {
        rtcmix_warn("CONTROLLER", "Controller number limited to 127");
        _controllerNumber = 127;
    }
    if (_controllerValue < 0.0) {
        rtcmix_warn("CONTROLLER", "Controller value limited to 0.0");
        _controllerValue = 0.0;
    }
    else if (_controllerValue > 1.0) {
        rtcmix_warn("CONTROLLER", "Controller value limited to 1.0");
        _controllerValue = 1.0;
    }
    
    PRINT("Chan %d ctrlr %d value %f (normalized)\n", _midiChannel, _controllerNumber, _controllerValue);

    return nSamps();
}

void CONTROLLER::doStart()
{
    unsigned value = unsigned(0.5 + (_controllerValue * 127));
    PRINT("doStart sending on chan %d: ctrlr %d value %u\n", _midiChannel, _controllerNumber, value);
    _outputPort->sendControl(0, _midiChannel, _controllerNumber, value);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void CONTROLLER::doupdate(FRAMETYPE currentFrame)
{
    // The Instrument base class update() function fills the <p> array with
    // the current values of all pfields.  There is a way to limit the values
    // updated to certain pfields.  For more about this, read
    // src/rtcmix/Instrument.h.
    
    double p[5];
    update(p, 5, 1 << 4);
    
    float newValue = p[4];
    if (newValue < 0.0) {
        rtcmix_warn("CONTROLLER", "Controller value limited to 0.0");
        newValue = 0;
    }
    else if (newValue > 1.0) {
        rtcmix_warn("CONTROLLER", "Controller value limited to 1.0");
        newValue = 1.0;
    }
    if (newValue != _controllerValue) {
        unsigned value = unsigned(0.5 + (newValue * 127));
        long timestamp = 1000.0 * (currentFrame - getRunStartFrame()) / SR;
        PRINT("doUpdate sending MIDI ctrlr %d with MIDI value %u with frame time offset %ld ms\n", _controllerNumber, value, timestamp);
        _outputPort->sendControl(timestamp, _midiChannel, _controllerNumber, value);
        _controllerValue = newValue;
    }
}

Instrument *makeCONTROLLER()
{
    CONTROLLER *inst = new CONTROLLER();
    inst->set_bus_config("CONTROLLER");
    
    return inst;
}

struct ctlr {
    int num;
    const char *name;
};

static const ctlr ctlrs[] = {
    {    0,              "bank select" },
    {    1,              "modulation" },
    {    2,              "breath controller" },
    {    4,              "foot controller" },
    {    5,              "portamento Time" },
    {    6,              "data entry MSB" },
    {    7,              "volume" },
    {    8,              "balance" },
    {    10,             "pan" },
    {    11,             "expression" },
    {    12,             "effect control 1" },
    {    13,             "effect control 2" },
    //{    32-63          LSB for Values 0-31 },
    {        64,             "sustain pedal" },
    {        65,             "portamento on/off" },
    {        66,             "sostenuto" },
    {        67,             "soft pedal" },
    {        68,             "legato footswitch" },
    {        69,             "hold 2" },
    {        70,             "sound controller 1" },          // (default: Sound Variation)
    {        71,             "sound controller 2" },           // (default: Timbre/Harmonic Content)
    {        72,             "sound controller 3" },             //  (default: Release Time)
    {        73,             "sound controller 4" },            //  (default: Attack Time)
    {        74,             "sound controller 5" },            //  (default: Brightness)
    {        84,             "portamento" },
    {        91,             "effects 1 depth" },
    {        92,             "effects 2 depth" },
    {        93,             "effects 3 depth" },
    {        94,             "effects 4 depth" },
    {        95,             "effects 5 depth" },
    {        96,             "data increment" },
    {        97,             "data decrement" },
    {        98,             "NRPN LSB" },
    {        99,             "NRPNumber MSB" },
    {        100,            "RPN LSB" },
    {        101,            "RPN MSB" },
    {       -1,              NULL }
};

extern "C"
double controller_number(float *p, int n_args, double *pp)
/* p1=controller name name */
{
    const char *key = DOUBLE_TO_STRING(pp[0]);
    
    if (key == NULL) {
        ::rterror("controller_number", "NULL controller name");
        return PARAM_ERROR;
    }

    for (const ctlr *c = &ctlrs[0]; c->name != NULL; ++c) {
        if (strcasestr(c->name, key) != NULL) {
            return (double) c->num;
        }
    }
    ::rterror("controller_number", "No matching name for ''%s'", key);
    return PARAM_ERROR;
}
