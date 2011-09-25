//
//  InputFile.h
//  RTcmixTest
//
//  Created by Douglas Scott on 9/24/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef INPUTDESC_H
#define INPUTDESC_H

#include "Lockable.h"
#include "rt_types.h"
#include <sys/types.h>

typedef int (*ReadFun)(int,int,int,off_t,long,BufPtr,int,int,const short[],short,void*);

/* definition of input file struct used by rtinput */
struct InputFile : public Lockable {
public:
	InputFile() : filename(NULL), fd(NO_FD), readBuffer(NULL) {}
    void init(int inFd, const char *inFileName, bool isAudioDev, int inHeaderType,
              int inDataFormat, int inDataLocation, long nFrames, float inSampleRate,
              int inChannels, double inDuration);
    
    off_t readSamps(off_t     cur_offset,       /* current file position before read */
                  BufPtr      dest,             /* interleaved buffer from inst */
                  int         dest_chans,       /* number of chans interleaved */
                  int         dest_frames,      /* frames in interleaved buffer */
                  const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                  /* (or NULL to fill all chans) */
                  short       src_chans         /* number of in-bus chans to copy */
    );
    
	char     *filename;         /* allocated by rtinput() */
	int      fd;                /* file descriptor, or NO_FD, or AUDIO_DEVICE */
	int      refcount;
	short    is_audio_dev;      /* true if input from audio dev, not from file */
	short    header_type;       /* e.g., AIFF_sound_file (in sndlib.h) */
	short    data_format;       /* e.g., snd_16_linear (in sndlib.h) */
	short    is_float_format;   /* true if data format is 32-bit float */
	int      data_location;     /* offset of sound data start in file */
	long     endbyte;           /* index of byte following last sample */
	float    srate;
	short    chans;
	double   dur;
    void*      readBuffer;
private:
    ReadFun    readFunction;
};

#endif
