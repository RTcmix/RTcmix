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
    Handle pvgetbin(const Arg args[], const int nargs);
}

static int gPvocFD = -1;
static bool gPvocNeedsSwap = 0;
static int gPvocDataOffset = 0;
static int gPvocFrameCount = 0;
static double gPvocFrameRate = 0;
static int gPvocBinsPerFrame = 0;       // how many amp/freq pairs

static const int kBinSizeInBytes = 8;   // 2 floats per bin

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
    gPvocBinsPerFrame = file_chans / 2;
    return gPvocFrameCount;
}

double pvgetbincount(const Arg args[], const int nargs)
{
    if (gPvocFD == -1) {
        return rtOptionalThrow(CONFIGURATION_ERROR);
    }
    return gPvocBinsPerFrame;
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
    if (gPvocFD == -1) {
        ::rterror("pvgetframe", "You haven't opened a PVOC data file yet");
        rtOptionalThrow(CONFIGURATION_ERROR);
        return NULL;
    }
    double frameToRead = (double)arglist[0];
    if (frameToRead > gPvocFrameCount-1.0) {
        rtcmix_warn("pvgetframe", "Limiting frame number to %f", gPvocFrameCount-1.0);
        frameToRead = gPvocFrameCount-1.0;
    }
    else if (frameToRead < 0.0) {
        return mkusage();
    }
    const int read_offset = gPvocDataOffset + ((int)frameToRead * (gPvocBinsPerFrame * kBinSizeInBytes));
    const int ampBins = gPvocBinsPerFrame;    // only want amplitudes
    
    // In PVOC datafiles, each frame consists of a float gain followed by a float
    // frequency.  We read entire frame, but only extract the gain bins here.
    
    if (lseek(gPvocFD, read_offset, SEEK_SET) == -1) {
        ::rterror("pvgetframe", "Failed to seek in data file");
        rtOptionalThrow(FILE_ERROR);
        return NULL;
    }
    float *totalframe = new float[gPvocBinsPerFrame*2];
    float *nextframe = new float[gPvocBinsPerFrame*2];

    // Create Array
    
    Array *outGains = (Array *)malloc(sizeof(Array));
    outGains->len = ampBins;
    outGains->data = (double *)malloc(ampBins * sizeof(double));

    read(gPvocFD, totalframe, gPvocBinsPerFrame*kBinSizeInBytes);
    read(gPvocFD, nextframe, gPvocBinsPerFrame*kBinSizeInBytes);

    double frac = frameToRead - (int)frameToRead;
    for (int bin = 0; bin < gPvocBinsPerFrame; ++bin) {
        float binVal = totalframe[bin*2];
        float nextBinVal = nextframe[bin*2];
        if (gPvocNeedsSwap) { byte_reverse4(&binVal); byte_reverse4(&nextBinVal); }
        outGains->data[bin] = binVal + frac * (nextBinVal - binVal);
    }
    delete [] totalframe;
    delete [] nextframe;
    
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outGains);
}

Handle
pvgetbin(const Arg arglist[], const int nargs)
{
    if (gPvocFD == -1) {
        ::rterror("pvgetbin", "You haven't opened a PVOC data file yet");
        rtOptionalThrow(CONFIGURATION_ERROR);
        return NULL;
    }
    double binToRead = (double)arglist[0];
    double maxBin = gPvocBinsPerFrame - 1;
    if (binToRead > maxBin) {
        rtcmix_warn("pvgetbin", "Limiting bin number to %f", maxBin);
        binToRead = maxBin;
    }
    else if (binToRead < 0.0) {
        die("pvgetbin", "Usage: pvgetbin(bin_number).  Must be between 0 and bin_count-1");
        rtOptionalThrow(PARAM_ERROR);
        return NULL;
    }
    
    // Offset to beginning of amplitude values for bin[binToRead] in the first PVOC frame
    int read_offset = gPvocDataOffset + int(binToRead * kBinSizeInBytes);
    if (lseek(gPvocFD, read_offset, SEEK_SET) == -1) {
        ::rterror("pvgetbin", "Failed to seek in data file");
        rtOptionalThrow(FILE_ERROR);
        return NULL;
    }
    
    // Create Array with length to hold an amp for each frame in the datafile
    
    Array *outGains = (Array *)malloc(sizeof(Array));
    outGains->len = gPvocFrameCount;
    outGains->data = (double *)malloc(gPvocFrameCount * sizeof(double));

    // How much we skip to get to the binToRead'th amplitude in each new frame.
    // This is the size of each frame minus the size of the float we read for the previous frame.
    int skipBytes = (gPvocBinsPerFrame * kBinSizeInBytes) - sizeof(float);
    // read amp bin, skip to next frame, read next amp bin, repeat
    for (int frame = 0; frame < gPvocFrameCount; ++frame) {
        float binamp = 0.0f;
        read(gPvocFD, &binamp, sizeof(float));
        if (gPvocNeedsSwap) { byte_reverse4(&binamp); }
//        printf("gain[%d] for frame %d: %.6f\n", (int)binToRead, frame, binamp);
        outGains->data[frame] = binamp;
        if (lseek(gPvocFD, skipBytes, SEEK_CUR) == -1) {
            free(outGains->data);
            free(outGains);
            ::rterror("pvgetbin", "Failed to seek in data file");
            rtOptionalThrow(FILE_ERROR);
            return NULL;
        }
    }
    
    // Wrap Array in Handle, and return.  This will return a 'list' to MinC.
    return createArrayHandle(outGains);
}
