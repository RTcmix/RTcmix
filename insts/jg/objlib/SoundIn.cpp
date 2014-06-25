/* __Experimental__ sound file input class, by John Gibson, 1999.
   Note also the various RawWav classes in STK.
*/

#include "SoundIn.h"


SoundIn :: SoundIn(char *fileName, double inskip = 0.0)
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

   fd = sndlib_open_read(fileName);
   if (fd == -1) {
      fprintf(stderr, "Can't read header of \"%s\" (%s)\n",
                                                  fileName, strerror(errno));
      exit(-1);                 // do something more graceful later...
   }

   /* Gather info from file header. */
   header_type = mus_header_type();
   if (NOT_A_SOUND_FILE(header_type)) {
      fprintf(stderr, "\"%s\" is probably not a sound file\n", fileName);
      sndlib_close(fd, 0, 0, 0, 0);
      exit(-1);                 // do something more graceful later...
   }
   data_format = mus_header_format();
   if (INVALID_DATA_FORMAT(data_format)) {
      fprintf(stderr, "\"%s\" has invalid sound data format\n", fileName);
      sndlib_close(fd, 0, 0, 0, 0);
      exit(-1);                 // do something more graceful later...
   }
   bytes_per_samp = mus_data_format_to_bytes_per_sample(data_format);
   srate = mus_header_srate();
   nchans = mus_header_chans();
   nframes = (long)(mus_header_samples() / nchans);
   data_location = mus_header_data_location();

   /* Allocate input float buffer. */
   bufframes = RTBUFSAMPS;   // NB: MUST be RTBUFSAMPS for read_samps to work
   inbuf = new BUFTYPE [nchans * bufframes];
   if (inbuf == NULL) {
      fprintf(stderr, "SoundIn can't allocate input buffer!\n");
      exit(-1);
   }

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
      outputs[n] = 0.0;
}


SoundIn :: ~SoundIn()
{
   delete [] inbuf;
   sndlib_close(fd, 0, 0, 0, 0);             // close, no update
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
double SoundIn :: getDuration()
{
   return ((double) nframes / (double) srate);
}


/* Reads in a new buffer using current file position pointer. If it reads
   past the end of file, pads buffer with zeros.
*/
int SoundIn :: readBuffer()
{
   int status;

   status = read_samps(fd, data_format, nchans, inbuf, nchans, bufframes);
   if (status != 0)
      return -1;

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

   byteoffset = data_location + (frame * nchans * bytes_per_samp);

   result = lseek(fd, byteoffset, SEEK_SET);

   return result;
}


/* Returns a pointer to a frame of floating-point samples (one per channel)
   for the current frame index, then increments the frame pointer.
   It's ok for this to continue reading past the end of file -- it'll just
   return 0's.
*/
double *SoundIn :: tick()
{
   if (curbufframe >= bufframes)
      readBuffer();

   BufPtr frame = &inbuf[curbufframe * nchans];
   for (int n = 0; n < nchans; n++)
      outputs[n] = frame[n];

   curbufframe++;                  // update for next time

   return outputs;
}


/* Returns result of last tick() for channel <chan>, which is a zero-based
   channel number.
   CAUTION: assumes <chan> is in range.
*/
double SoundIn :: lastOutput(int chan)
{
   return outputs[chan];
}


