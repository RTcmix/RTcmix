/* __Experimental__ sound file input class, by John Gibson, 1999.
   Note also the various RawWav classes in STK.
*/

#include "SoundIn.h"


SoundIn :: SoundIn(char *fileName, MY_FLOAT inskip = 0.0)
{
   struct stat st;

   /* See if file exists and is a regular file or link. */
   if (stat(fileName, &st) == -1) {
      fprintf(stderr, "%s: %s\n", fileName, strerror(errno));
      exit(-1);                 // do something more graceful later...
   }
   if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)) {
      fprintf(stderr, "\"%s\" is not a regular file or a link.\n", fileName);
      exit(-1);                 // do something more graceful later...
   }

#ifdef USE_SNDLIB
   fd = sndlib_open_read(fileName);
   if (fd == -1) {
      fprintf(stderr, "Can't read header of \"%s\" (%s)\n",
                                                  fileName, strerror(errno));
      exit(-1);                 // do something more graceful later...
   }

   /* Gather info from file header. */
   header_type = c_snd_header_type();
   if (NOT_A_SOUND_FILE(header_type)) {
      fprintf(stderr, "\"%s\" is probably not a sound file\n", fileName);
      sndlib_close(fd, 0, 0, 0, 0);
      exit(-1);                 // do something more graceful later...
   }
   data_format = c_snd_header_format();
   if (INVALID_DATA_FORMAT(data_format)) {
      fprintf(stderr, "\"%s\" has invalid sound data format\n", fileName);
      sndlib_close(fd, 0, 0, 0, 0);
      exit(-1);                 // do something more graceful later...
   }
   srate = c_snd_header_srate();
   nchans = c_snd_header_chans();
   nframes = (long)(c_snd_header_data_size() / nchans);
   data_location = c_snd_header_data_location();

   /* Allocate array of 1-channel I/O buffers. */
   bufframes = RTBUFSAMPS;
   inbufs = sndlib_allocate_buffers(nchans, bufframes);
   if (inbufs == NULL) {
      fprintf(stderr, "SoundIn can't allocate sndlib input buffers!\n");
      exit(-1);                 // do something more graceful later...
   }
#else /* !USE_SNDLIB */
   fprintf(stderr, "Sorry, SoundIn not implemented yet!\n");
   exit(-1);
#endif /* !USE_SNDLIB */

   if (inskip > 0.0) {
      long frame = (long)(inskip * srate + 0.5);
      if (frame >= nframes) {
         fprintf(stderr, "SoundIn: inskip greater than file duration.\n");
         exit(-1);                // do something more graceful later...
      }
      curbufstart = -1;           // force seekFrame to seek
      seekFrame(frame);
   }
   else {   // NOTE: ok if inskip negative
      curbufstart = -bufframes;   // compensate for increment in 1st readBuffer
      curbufframe = bufframes;    // force next tick to read new buffer
   }

   for (int n = 0; n < MAXCHANS; n++)
      outputs[n] = (MY_FLOAT) 0.0;
}


SoundIn :: ~SoundIn()
{
#ifdef USE_SNDLIB
   sndlib_free_buffers(inbufs, nchans);
   sndlib_close(fd, 0, 0, 0, 0);             // close, no update
#endif
}


/* Returns the number of channels in the file. */
int SoundIn :: getChannels()
{
   return nchans;
}


/* Returns the sampling rate of the file. */
int SoundIn :: getSamplingRate()
{
   return srate;
}


/* Returns the number of frames in the file. */
long SoundIn :: getFrames()
{
   return nframes;
}


/* Returns the duration of the file if played at its sampling rate. */
MY_FLOAT SoundIn :: getDuration()
{
   return ((MY_FLOAT)nframes / (MY_FLOAT)srate);
}


/* Reads in a new buffer using current file position pointer. If it reads
   past the end of file, pads buffer with zeros.
*/
int SoundIn :: readBuffer()
{
#ifdef USE_SNDLIB
   clm_read(fd, 0, bufframes - 1, nchans, inbufs);
// ***NOTE: doesn't return an error code, so we don't either!
#endif

   curbufstart += bufframes;
   curbufframe = 0;

   return 0;
}


/* Seeks to the given frame, which must be in the range [0, nframes).
   Updates curbufframe so that the next tick() will do the right thing.
   Returns -1 if error, 0 if OK.
*/
int SoundIn :: seekFrame(long frame)
{
   long byteoffset, result = 0;

   assert(frame >= 0 && frame < nframes);

   /* If <frame> is in current buffer, no need to seek and read;
      just change curbufframe index.
   */
   if (frame >= curbufstart && frame < curbufstart + bufframes) {
      curbufframe = frame - curbufstart;
      return 0;
   }

   curbufstart = frame - bufframes;     // compensate for next readBuffer incr.
   curbufframe = bufframes;             // force next tick to read new buffer

   /* NOTE: clm_seek handles any datum size as if it had 16 bits. */
   byteoffset = data_location + (frame * nchans * 2);

#ifdef USE_SNDLIB
   result = clm_seek(fd, byteoffset, SEEK_SET);
#endif

   return result;
}


/* Returns a pointer to a frame of floating-point samples (one per channel)
   for the current frame index, then increments the frame pointer.
   It's ok for this to continue reading past the end of file -- it'll just
   return 0's.
*/
MY_FLOAT *SoundIn :: tick()
{
   if (curbufframe >= bufframes)
      readBuffer();

   for (int n = 0; n < nchans; n++)
      outputs[n] = (MY_FLOAT) inbufs[n][curbufframe];

   curbufframe++;                  // update for next time

   return outputs;
}


/* Returns result of last tick() for channel <chan>, which is a zero-based
   channel number.
   CAUTION: assumes <chan> is in range.
*/
MY_FLOAT SoundIn :: lastOutput(int chan)
{
   return (MY_FLOAT)inbufs[chan][curbufframe];
}


