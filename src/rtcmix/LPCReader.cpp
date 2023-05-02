//
//  LPCReader.cpp
//
//  Created by Douglas Scott on 5/1/23.
//

#include "lpcdefs.h"
#include <ugens.h>        // for warn, die
#include <rtcmix_types.h>
#include "utils.h"
#include "DataSet.h"
#include <stdlib.h>
#include <string.h>

extern int GetDataSet(DataSet **);

extern "C" {
    Handle lpcgetpitches(const Arg args[], const int nargs);
}

Handle lpcgetpitches(const Arg args[], const int nargs)
{
    DataSet *lpcdata = NULL;
    GetDataSet(&lpcdata);
    if (lpcdata == NULL) {
        ::rterror("lpcgetpitches", "You haven't opened an LPCdata file yet");
        rtOptionalThrow(CONFIGURATION_ERROR);
        return NULL;
    }

    lpcdata->ref();
    int frameCount = (int)lpcdata->getFrameCount();
    
    int firstFrame = (int)(double)args[0];
    int lastFrame = (int)(double)args[1];
    if (firstFrame >= frameCount) {
        ::rterror("lpcgetpitches", "Start frame is beyond end of file");
        rtOptionalThrow(CONFIGURATION_ERROR);
        return NULL;
    }
    if (lastFrame >= frameCount) {
        rtcmix_warn("lpcgetpitches", "End frame is beyond end of file - truncating");
        lastFrame = frameCount - 1;
    }
    if (firstFrame >= lastFrame) {
        ::rterror("lpcgetpitches", "End frame must be > start frame");
        rtOptionalThrow(CONFIGURATION_ERROR);
        return NULL;
    }
    
    int framesToRead = lastFrame - firstFrame + 1;

    Array *outPitches = (Array *)malloc(sizeof(Array));
    outPitches->len = framesToRead;
    outPitches->data = (double *)malloc(framesToRead * sizeof(double));
    memset(outPitches->data, 0, framesToRead * sizeof(double));
    float coeffs[MAXPOLES+4];
    for (int i = firstFrame; i <= lastFrame; ++i) {
        if (lpcdata->getFrame((double) i, coeffs) < 0)
            break;
        outPitches->data[i-firstFrame] = coeffs[PITCH];
    }

    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outPitches);
}
