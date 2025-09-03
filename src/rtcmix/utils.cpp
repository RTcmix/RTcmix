//
// Created by Douglas Scott on 1/20/24.
//

#include "rtcmix_types.h"
#include "ugens.h"
#include "handle.h"
#include <math.h>
#ifdef linux
#include <limits.h>
#endif

extern "C" {
    Handle m_bits(const Arg args[], int nargs);
}

Handle m_bits(const Arg args[], const int nargs) {
    if (nargs != 1 && nargs != 2) {
        rterror("bits", "Usage: bits(number [,bitfield_len])");
        rtOptionalThrow(PARAM_ERROR);
        return NULL;
    };
    unsigned number = (unsigned)(int) (double) args[0];
    int numBits = 0;
    if (nargs == 2)
        numBits = (int) args[1];
    else {
        if (number < 32768)
            numBits = 16;
        else if (number < INT_MAX)
            numBits = 32;
        else numBits = 64;
    }
    // Create Array

    Array *outBits = (Array *)malloc(sizeof(Array));
    outBits->len = numBits;
    outBits->data = (double *)malloc(numBits * sizeof(double));

    unsigned bit = 1;
    for (int n = 0; n < numBits; ++n) {
        outBits->data[n] = (number & bit) != 0;
        bit <<= 1;
    }
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outBits);
}
