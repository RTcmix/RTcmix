/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include <stdlib.h>
#include <stdio.h>
#include <RTOption.h>
#include "../../../src/rtcmix/DynamicLib.h"
#include <RTMIDIOutput.h>
#include <ugens.h>
#include <rt.h>

typedef void * (*MIDIOutputCreator)();

static RTMIDIOutput *gMIDIOutput;

RTMIDIOutput *getMIDIOutput()
{
    return gMIDIOutput;
}

double
setup_midi(float *p, int n_args)
{
#ifndef EMBEDDED
    char loadPath[1024];
    const char *dsoPath = RTOption::dsoPath();
    if (strlen(dsoPath) == 0) {
        dsoPath = SHAREDLIBDIR;
    }
    sprintf(loadPath, "%s/libmidiconn.so", dsoPath);

    DynamicLib theDSO;
    if (theDSO.load(loadPath) == 0) {
        MIDIOutputCreator creator = NULL;
        if (theDSO.loadFunction(&creator, "create_midi_output") == 0) {
            // Pass 2nd thru last args, leaving off selector
            gMIDIOutput = (RTMIDIOutput *)(*creator)();
            if (gMIDIOutput == NULL) {
                return rtOptionalThrow(SYSTEM_ERROR);
            }
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

double controller_number(float *p, int n_args, double *pp);

int profile()
{
    UG_INTRO("setup_midi", setup_midi);
    UG_INTRO("controller_number", controller_number);
    return 0;
}

}

extern Instrument *makePITCHBEND();
extern Instrument *makeCONTROLLER();
extern Instrument *makePROGRAM();
extern Instrument *makeNOTE();

void rtprofile()
{
    RT_INTRO("PITCHBEND", makePITCHBEND);
    RT_INTRO("CONTROLLER", makeCONTROLLER);
    RT_INTRO("PROGRAM", makePROGRAM);
    RT_INTRO("NOTE", makeNOTE);
}

#endif

