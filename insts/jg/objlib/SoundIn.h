/* __Experimental__ sound file input class, by John Gibson, 1999.
   Note also the various RawWav classes in STK.
*/
#if !defined(__SoundIn_h)
#define __SoundIn_h

#include "objdefs.h"
#include <sys/stat.h>
#include <sndlibsupport.h>

class SoundIn
{
  protected:
    int      fd;
    int      nchans;
    int      srate;
    int      header_type;      // header type, e.g., NeXT, AIFF (see sndlib.h)
    int      data_format;      // sample format (see sndlib.h)
    int      data_location;    // offset in bytes of sound data
    long     nframes;          // number of frames in file
    int      bufframes;        // number of frames in buffer
    long     curbufstart;      // index of first buffer frame, relative to
                               //    start of sound data
    int      curbufframe;      // frame index within current buffer
    int      **inbufs;         // sndlib-style array of 1-channel buffers
    MY_FLOAT outputs[MAXCHANS];

    int readBuffer();

  public:
    SoundIn(char *fileName, MY_FLOAT inskip);
    ~SoundIn();
    int getChannels();
    int getSamplingRate();
    long getFrames();
    MY_FLOAT getDuration();
    int seekFrame(long frame);
    MY_FLOAT *tick();
    MY_FLOAT lastOutput(int chan);
};

#endif
