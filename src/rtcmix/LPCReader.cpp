//
//  LPCReader.cpp
//
//  Created by Douglas Scott on 5/1/23.
//

#include "lpcdefs.h"
#include <ugens.h>        // for warn, die
#include <rtcmix_types.h>
#include "handle.h"
#include "LPCDataSet.h"
#include <stdlib.h>
#include <string.h>

extern "C" {
    Handle lpcgetpitches(const Arg args[], int nargs);
    Handle lpcgetamps(const Arg args[], int nargs);
}

Handle getlpcframedata(const char *filename, const char *functionname, int npolesGuess, int frameField, int firstFrame, int lastFrame, float thresh)
{
    LPCDataSet *dataSet = new LPCDataSet;
    int frms = (int) dataSet->open(filename, npolesGuess, 0);   // note: this will fail if file has no header
    try {
        if (frms < 0) {
            ::rterror(functionname, "Failed to open LPC file");
            throw FILE_ERROR;
        }

        dataSet->ref();
        int frameCount = (int) dataSet->getFrameCount();

        rtcmix_advise(functionname, "Data has %d total frames.", frameCount);

        if (firstFrame >= frameCount) {
            ::rterror(functionname, "Start frame is beyond end of file");
            throw PARAM_ERROR;
        }
        if (lastFrame >= frameCount) {
            rtcmix_warn(functionname, "End frame is beyond end of file - truncating");
            lastFrame = frameCount - 1;
        }
        if (firstFrame >= lastFrame) {
            ::rterror(functionname, "End frame must be > start frame");
            throw PARAM_ERROR;
        }
    } catch (RTcmixStatus status) {
        dataSet->unref();
        rtOptionalThrow(status);
        return NULL;
    }

    // If no threshold entered, pass all frame data values.

    int framesToRead = lastFrame - firstFrame + 1;

    Array *outValues = (Array *)malloc(sizeof(Array));
    outValues->len = framesToRead;
    outValues->data = (double *)malloc(framesToRead * sizeof(double));
    memset(outValues->data, 0, framesToRead * sizeof(double));
    float coeffs[MAXPOLES+4];
    for (int i = firstFrame; i <= lastFrame; ++i) {
        if (dataSet->getFrame((double) i, coeffs) < 0)
            break;
        outValues->data[i-firstFrame] = (coeffs[THRESH] < thresh) ? coeffs[frameField] : 0.0;
    }
    rtcmix_advise(functionname, "Returning list of %d values", framesToRead);
    
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outValues);
}

// lpcgetpitches(filename, npoles_guess, first_frame, last_frame [, err_threshold])
// Open an LPC datafile and return the pitch frame values as a list object.
// Any frame whose error value equals or exceeds err_threshold will have its pitch set to zero.
// This allows you to get a list of "valid" pitches for the given threshold.

Handle lpcgetpitches(const Arg args[], const int nargs)
{
    const char *filename = args[0];
    const int npolesGuess = (int) (double) args[1];
    int firstFrame = (int) (double) args[2];
    int lastFrame = (int) (double) args[3];
    float errThreshold = (nargs < 5) ? 1.0 : (float)args[4];
    return getlpcframedata(filename, "lpcgetpitches", npolesGuess, PITCH, firstFrame, lastFrame, errThreshold);
}

// lpcgetamps(filename, npoles_guess, first_frame, last_frame [, err_threshold])
// Open an LPC datafile and return the amplitude values as a list object.
// Any frame whose error value equals or exceeds err_threshold will have its amp set to zero.
// This allows you to treat frames with high error values as muted.

Handle lpcgetamps(const Arg args[], const int nargs)
{
    const char *filename = args[0];
    const int npolesGuess = (int) (double) args[1];
    int firstFrame = (int) (double) args[2];
    int lastFrame = (int) (double) args[3];
    float errThreshold = (nargs < 5) ? 1.0 : (float)args[4];
    return getlpcframedata(filename, "lpcgetamps", npolesGuess, RESIDAMP, firstFrame, lastFrame, errThreshold);
}
