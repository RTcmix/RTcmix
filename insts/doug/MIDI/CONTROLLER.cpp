//
//  CONTROLLER.cpp
//  RTcmix Desktop
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = amplitude multiplier
 p3 = MIDI channel
 p4 = Controller number (integer)
 p5 = Controller value (0.0 - 1.0)
*/

#include "CONTROLLER.h"
#include "RTMIDIOutput.h"
#include <ugens.h>
#include <rt.h>
#include <stdio.h>
#include <string.h>

CONTROLLER::CONTROLLER() : MIDIBase(), _controllerNumber(-1), _controllerValue(0.0)
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
    _controllerNumber = (int)(p[4]);     // midipch returns a float, so make sure we round appropriately
    _controllerValue = p[5];

//    printf("Chan %d, controller %d, value %f\n", _midiChannel, _controllerNumber, _controllerValue);

    return nSamps();
}

void CONTROLLER::doStart()
{
    unsigned value = unsigned(0.5 + (_controllerValue * 127));
//    printf("Sending MIDI ctrlr %d with MIDI value %u\n", _controllerNumber, value);
    _outputPort->sendControl(0, _midiChannel, _controllerNumber, value);
}

// Called at the control rate to update parameters like amplitude, pan, etc.

void CONTROLLER::doupdate(FRAMETYPE currentFrame)
{
    // The Instrument base class update() function fills the <p> array with
    // the current values of all pfields.  There is a way to limit the values
    // updated to certain pfields.  For more about this, read
    // src/rtcmix/Instrument.h.
    
    double p[6];
    update(p, 6, 1 << 5);
    
    float newValue = p[5];
    if (newValue != _controllerValue) {
        unsigned value = unsigned(0.5 + (newValue * 127));
        long timestamp = 1000.0 * (currentFrame - getRunStartFrame()) / SR;
//        printf("Sending MIDI ctrlr %d with MIDI value %u with frame time offset %ld ms\n", _controllerNumber, value, timestamp);
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
