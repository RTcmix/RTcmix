/* __Experimental__ sound file input class, by John Gibson, 1999.
   Note also the various RawWav classes in STK.
*/
#if !defined(__SoundIn_h)
#define __SoundIn_h

#include "objdefs.h"
#include <sys/stat.h>
#include <sndlibsupport.h>
#include <buffers.h>          /* for BUFTYPE and BufPtr */
                              /* NB: assumes BUFTYPE is same as double */
#include <prototypes.h>       /* for read_samps */

class SoundIn
{
  protected:
    int      fd;
    int      nchans;
    int      srate;
    int      header_type;      // header type, e.g., NeXT, AIFF (see sndlib.h)
    int      data_format;      // sample format (see sndlib.h)
    int      bytes_per_samp;
    int      data_location;    // offset in bytes of sound data
    long     nframes;          // number of frames in file
    int      bufframes;        // number of frames in buffer
    long     curbufstart;      // index of first buffer frame, relative to
                               //    start of sound data
    int      curbufframe;      // frame index within current buffer
    BufPtr   inbuf;            // interleaved array of nchans channels
    double   outputs[MAXCHANS];

    int readBuffer();

  public:
    SoundIn(char *fileName, double inskip);
    ~SoundIn();
    int getChannels();
    int getSamplingRate();
    long getFrames();
    double getDuration();
    int seekFrame(long frame);
    double *tick();
    double lastOutput(int chan);
};

#endif
