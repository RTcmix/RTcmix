//
//  PvocReader.cpp
//
//  Created by Douglas Scott on 8/26/21.
//

#include <ugens.h>        // for warn, die
#include "byte_routines.h"
#include "prototypes.h"
#include "sfheader.h"
#include "sndlibsupport.h"
#include "utils.h"
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits>

extern "C" {
    double pvinput(const Arg args[], const int nargs);
    double pvgetbincount(const Arg args[], const int nargs);
    double pvgetframerate(const Arg args[], const int nargs);
    Handle pvgetframe(const Arg args[], const int nargs);
}

Handle
createArrayHandle(Array *array)
{
    Handle handle = (Handle) malloc(sizeof(struct _handle));
    if (handle) {
        handle->type = ListType;        // indicates we are returning an object to be referenced as a list
        handle->ptr = (void *) array;
        handle->refcount = 0;
    }
    return handle;
}

static int gPvocFD = -1;
static bool gPvocNeedsSwap = 0;
static int gPvocDataOffset = 0;
static int gPvocFrameCount = 0;
static double gPvocFrameRate = 0;
static int gPvocBinsPerFrame = 0;

double
pvinput(const Arg arglist[], const int nargs)
{
    if (gPvocFD > 0) {
        close(gPvocFD);
        gPvocFD = -1;   // mark as closed
    }
    const char *dataFileName = (const char *) arglist[0];
    if (!dataFileName) {
        die("pvinput", "Usage: pvinput(\"pvoc_data_filename\")");
        rtOptionalThrow(PARAM_ERROR);
        return -1;
    }

    int data_format, data_location, file_chans;
    long file_samps;
    double srate;
    
    int fd = open_sound_file("pvinput", dataFileName, NULL,
                             &data_format, &data_location, &srate, &file_chans, &file_samps);
    if (fd == -1) {
        return -1;
    }
    
    int file_frames = int(file_samps / file_chans);

    if (file_chans == 0 || file_chans % 2 != 0) {
        ::rterror("pvinput", "Data must be stored in a non-zero, even number of channels");
        return rtOptionalThrow(PARAM_ERROR);
    }
    rtcmix_advise("pvinput", "format: %d, data offset: %d, total channels: %d, total frames: %ld, frame rate: %f",
                  data_format, data_location, file_chans, file_frames, srate);
    
    if (!IS_FLOAT_FORMAT(data_format)) {
        ::rterror("pvinput", "Frame data must be in floating point format");
        return rtOptionalThrow(PARAM_ERROR);
    }
    
#if MUS_LITTLE_ENDIAN
    gPvocNeedsSwap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
    gPvocNeedsSwap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif

    gPvocFD = fd;
    gPvocDataOffset = data_location;
    gPvocFrameCount = file_frames;
    gPvocFrameRate = srate;
    gPvocBinsPerFrame = file_chans;
    return gPvocFrameCount;
}

double pvgetbincount(const Arg args[], const int nargs)
{
    if (gPvocFD == -1) {
        return rtOptionalThrow(CONFIGURATION_ERROR);
    }
    return gPvocBinsPerFrame / 2.0;
}

double pvgetframerate(const Arg args[], const int nargs)
{
    if (gPvocFD == -1) {
        return rtOptionalThrow(CONFIGURATION_ERROR);
    }
    return (double)gPvocFrameRate;
}

static Handle mkusage()
{
    die("pvgetframe", "Usage: pvgetframe(frame_number).  Must be between 0.0 and framecount-1");
    rtOptionalThrow(PARAM_ERROR);
    return NULL;
}

Handle
pvgetframe(const Arg arglist[], const int nargs)
{
    double frameToRead = (double)arglist[0];
    if (frameToRead > gPvocFrameCount-1.0) {
        rtcmix_warn("pvgetframe", "Limiting frame number to %f", gPvocFrameCount-1.0);
        frameToRead = gPvocFrameCount-1.0;
    }
    else if (frameToRead < 0.0) {
        return mkusage();
    }
    int read_offset = gPvocDataOffset + (int)frameToRead * (gPvocBinsPerFrame * sizeof(float));
    int ampBins = gPvocBinsPerFrame / 2;    // only want amplitudes
    
    // In PVOC datafiles, each frame consists of a float gain followed by a float
    // frequency.  We just pull the gain bins for now
    
    if (lseek(gPvocFD, read_offset, SEEK_SET) == -1) {
        ::rterror("pvgetframe", "Failed to seek in data file");
        rtOptionalThrow(FILE_ERROR);
        return NULL;
    }
    float *totalframe = new float[gPvocBinsPerFrame];
    float *nextframe = new float[gPvocBinsPerFrame];

    // Create Array
    
    Array *outGains = new Array;
    outGains->len = ampBins;
    outGains->data = (double *)malloc(ampBins * sizeof(double));

    read(gPvocFD, totalframe, gPvocBinsPerFrame*sizeof(float));
    read(gPvocFD, nextframe, gPvocBinsPerFrame*sizeof(float));

    double frac = frameToRead - (int)frameToRead;
    for (int bin = 0, loc = 0; bin < gPvocBinsPerFrame; bin += 2, ++loc) {
        float binVal = totalframe[bin];
        float nextBinVal = nextframe[bin];
        if (gPvocNeedsSwap) { byte_reverse4(&binVal); byte_reverse4(&nextBinVal); }
        outGains->data[loc] = binVal + frac * (nextBinVal - binVal);
    }
    delete [] totalframe;
    delete [] nextframe;
    
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outGains);
}
