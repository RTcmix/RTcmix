/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include <stdlib.h>
#include <stdio.h>
#include <RTOption.h>
#include "../../../src/rtcmix/DynamicLib.h"
#include <RTMIDIOutput.h>
#include <RTcmix.h>
#include <ugens.h>
#include <rt.h>

const int kMaxMIDIPorts = 4;

typedef void * (*MIDIOutputCreator)(const char *);

static RTMIDIOutput *gMIDIOutputs[kMaxMIDIPorts];
static int gMIDIOutputPortCount;

RTMIDIOutput *getMIDIOutputForChannel(int channel)
{
    int portIndex = channel / 16;
    if (portIndex < gMIDIOutputPortCount) {
        return gMIDIOutputs[portIndex];
    }
    return NULL;
}

void outputDestroyCallback(void *context)
{
    RTMIDIOutput **outputs = gMIDIOutputs;
    for (int i = 0; i < gMIDIOutputPortCount; ++i) {
        gMIDIOutputs[i] = NULL;
    }
    for (int i = 0; i < gMIDIOutputPortCount; ++i) {
        delete outputs[i];
    }
    gMIDIOutputPortCount = 0;
}

// setup_midi([["MIDI_Port1_Name"],"MIDI_Port2_Name", ...])

double
setup_midi(double *p, int n_args)
{
#ifndef EMBEDDED
    if (gMIDIOutputPortCount > 0) {
        return 1;   // Don't allow repeated calls
    }
    char loadPath[1024];
    const char *dsoPath = RTOption::dsoPath();
    if (strlen(dsoPath) == 0) {
        dsoPath = SHAREDLIBDIR;
    }
    snprintf(loadPath, 1024, "%s/libmidiconn.so", dsoPath);

    int portCount;
    const char *portnames[4];

    if (n_args == 0) {
        portCount = 1;
        portnames[0] = NULL;
    }
    else {
        portCount = std::min(kMaxMIDIPorts, n_args);
        for (int n = 0; n < portCount; ++n) {
            portnames[n] = DOUBLE_TO_STRING(p[n]);
            if (portnames[n] == NULL) {
                die("setup_midi", "If a port name is specified, it cannot be NULL");
                return rtOptionalThrow(PARAM_ERROR);
            }
        }
    }
    DynamicLib theDSO;
    if (theDSO.load(loadPath) == 0) {
        MIDIOutputCreator creator = NULL;
        if (theDSO.loadFunction(&creator, "create_midi_output") == 0) {
            for (int pidx = 0; pidx < portCount; ++pidx) {
                gMIDIOutputs[pidx] = (RTMIDIOutput *)(*creator)(portnames[pidx]);
                if (gMIDIOutputs[pidx] == NULL) {
                    return rtOptionalThrow(SYSTEM_ERROR);
                }
            }
            rtcmix_advise("setup_midi", "Created %d MIDI output(s)", portCount);
            gMIDIOutputPortCount = portCount;
            // This will cause the MIDIOutput to be destroyed when the system is.
            RTcmix::registerDestroyCallback(outputDestroyCallback, NULL);
        }
        else {
            theDSO.unload();
            die("setup_midi", "symbol lookup failed: %s\n", theDSO.error());
            return rtOptionalThrow(SYSTEM_ERROR);
        }
    }
    else {
        die("setup_midi", "dso load failed: %s\n", theDSO.error());
        return rtOptionalThrow(SYSTEM_ERROR);
    }
    return 1;
#else
    rterror("setup_midi", "MIDI output not supported in this configuration");
    return rtOptionalThrow(SYSTEM_ERROR);
#endif
}

#ifndef EMBEDDED

class Instrument;

extern "C" {

double controller_number(double p[], int n_args);

int profile()
{
    UG_INTRO("setup_midi", setup_midi);
    UG_INTRO("controller_number", controller_number);
    return 0;
}

}

extern Instrument *makeSYSEX();
extern Instrument *makePITCHBEND();
extern Instrument *makeCONTROLLER();
extern Instrument *makePROGRAM();
extern Instrument *makeNOTE();

void rtprofile()
{
    RT_INTRO("SYSEX", makeSYSEX);
    RT_INTRO("PITCHBEND", makePITCHBEND);
    RT_INTRO("CONTROLLER", makeCONTROLLER);
    RT_INTRO("PROGRAM", makePROGRAM);
    RT_INTRO("NOTE", makeNOTE);
}

#endif

