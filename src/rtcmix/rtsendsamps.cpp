/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* rev'd for v2.3, JGG, 20-Feb-00 */
/* major revision for 3.7 (converted to C++ source), DS, April-04 
   (see deleted .c file for earlier history)
*/

#include <stdio.h>
#include <unistd.h>
#include <math.h>       /* for fabs */
#include <assert.h>
#include <globals.h>
#include <prototypes.h>
#include <buffers.h>
#include <ugens.h>
#include <sndlibsupport.h>
#include "rtdefs.h"
#include <AudioDevice.h>
#include <AudioFileDevice.h>
#include "Option.h"

/* #define DUMP_AUDIO_TO_RAW_FILE */
/* #define USE_REAL2INT */

extern AudioDevice *globalOutputFileDevice;	// audio_devices.cpp

static int printing_dots = 0;

/* local prototypes */
static int write_to_audio_device(AudioDevice *);
static void limiter(void);

#ifdef DUMP_AUDIO_TO_RAW_FILE
/* ----------------------------------------------- dump_audio_to_raw_file --- */
/* For debugging audio device writes.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static void
dump_audio_to_raw_file(OBufPtr dest, int nbytes)
{
   int         result;
   static int  fd = -1;

   if (fd == -1) {
      fd = open("dumpaudio.raw", O_RDWR | O_CREAT | O_TRUNC, 0666);
      assert(fd > 0);
      fprintf(stderr, "Dumping audio to \"dumpaudio.raw\".\n");
   }
   result = write(fd, dest, nbytes);
   assert(result != -1);
}
#endif /* DUMP_AUDIO_TO_RAW_FILE */

#ifdef USE_REAL2INT
/* ------------------------------------------------------------- real2int --- */
/* This is supposed to be more efficient than casting from float to int,
   but I didn't notice much difference.  This came from info off of the
   music-dsp list.  -JGG
*/
typedef long int32;
static double _double2fixmagic = 68719476736.0 * 1.5;
static int32 _shiftamt = 16;
#if MUS_LITTLE_ENDIAN    /* processor is little-endian */
   #define iman_   0
#else
   #define iman_   1
#endif

static INLINE int
real2int(float val)
{
   double x = (double) val;
   x = x + _double2fixmagic;
   return ((int32 *) &x)[iman_] >> _shiftamt;
}
#endif /* USE_REAL2INT */


/* ------------------------------------------------ write_to_audio_device --- */
static int
write_to_audio_device(AudioDevice *device)
{
	return device->sendFrames(out_buffer, RTBUFSAMPS) == RTBUFSAMPS ? 0 : -1;
}


/* -------------------------------------------------------------- limiter --- */
/* If clipping occurs within the out_buffer, pin the clipped samples to
   extrema and then print some information about them: how many samples
   clipped, the absolute value of the sample farthest out of range, the
   time range of this buffer in the output, etc. Also, maintain peak
   amplitude stats for later reporting and writing to the comment of the
   output file (if any).

   Note: This modifies the floating-point output buffers *in place*.
*/
static void
limiter()
{
   int      i, j, numclipped;
   BUFTYPE  clipmax, orig_samp;
   BufPtr   buf;

   numclipped = 0;
   clipmax = (BUFTYPE) 0;

   for (i = 0; i < NCHANS; i++) {
      buf = out_buffer[i];
      for (j = 0; j < RTBUFSAMPS; j++) {
         orig_samp = buf[j];
         if (orig_samp < -32768.0) {
            if (orig_samp < -clipmax)
               clipmax = -orig_samp;
            buf[j] = -32768.0;
            numclipped++;
         }
         else if (orig_samp > 32767.0) {
            if (orig_samp > clipmax)
               clipmax = orig_samp;
            buf[j] = 32767.0;
            numclipped++;
         }
         if (Option::checkPeaks()) {
            BUFTYPE abs_samp = (BUFTYPE) fabs((double) buf[j]);
            if (abs_samp > peaks[i]) {
               peaks[i] = abs_samp;
               peaklocs[i] = bufStartSamp + (j / NCHANS);  /* frame count */
            }
         }
      }
   }

   if (numclipped && Option::reportClipping()) {
      float loc1 = (float) bufStartSamp / SR;
      float loc2 = loc1 + ((float) RTBUFSAMPS / SR);

      /* We start with a newline if we're also printing the buffer dots. */
      fprintf(stderr,
              "%s  CLIPPING: %4d samps, max: %g, time range: %f - %f\n",
              (printing_dots? "\n" : ""), numclipped, clipmax, loc1, loc2);
   }

//printf("limiter: bufStartSamp = %ld\n", bufStartSamp);
}


/* ---------------------------------------------------------- rtsendzeros --- */
/* Send a buffer of zeros to the audio output device, and to the output sound
   file if <also_write_to_file> is true.

   NOTE NOTE NOTE: This clears all the global out_buffers!
*/
void
rtsendzeros(AudioDevice *device, int also_write_to_file)
{
   int   err;

   clear_output_buffers();

   if (Option::play()) {
      int i, j, nsamps, nbufs;
      err = write_to_audio_device(device);
      if (err)
         fprintf(stderr, "rtsendzeros: Error: %s\n", device->getLastError());
   }

   if (also_write_to_file && rtfileit) {
      err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendzeros: bad write to output sound file\n");
   }
}


/* ---------------------------------------------------------- rtsendsamps --- */
/* Called by the scheduler to write the output buffer to the audio device
   and/or a sound file.   All format conversion happens inside the AudioDevice.
   For all supported cases, we limit the floating point buffer to +-32768 to
   avoid overflow during conversion to other formats.
*/

void
rtsendsamps(AudioDevice *device)
{
   int   err;

   /* If we're writing to a file, and not playing, print a dot to show
      we've output one buffer.
   */
   if (!Option::play() && rtfileit) {
      printing_dots = 1;
      printf(".");    /* no '\n' */
   }

   /* Write float file *before* doing any limiting. */
   if (is_float_format && rtfileit) {
	  // FOR NOW, IF WE ARE BOTH PLAYING AND WRITING, DO IT WITH SEPARATE
	  // AudioDevice INSTANCES.
	  if (Option::play())
      	err = rtwritesamps(globalOutputFileDevice);
	  else
	  	err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendsamps: bad write to output sound file\n");
      if (!Option::play())
         return;        /* without limiting */
   }

   limiter();    /* Limit output buffer data to +-32767.0 */

   if (Option::play()) {
      err = write_to_audio_device(device);
      if (err)
         fprintf(stderr, "rtsendsamps: Error: %s\n", device->getLastError());
   }

   if (!is_float_format && rtfileit) {
	  // FOR NOW, IF WE ARE BOTH PLAYING AND WRITING, DO IT WITH SEPARATE
	  // AudioDevice INSTANCES.
	  if (Option::play())
      	err = rtwritesamps(globalOutputFileDevice);
	  else
	  	err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendsamps: bad write to output sound file\n");
   }
}


/* -------------------------------------------------------- rtreportstats --- */
void
rtreportstats(AudioDevice *device)
{
	int    n;
	double dbref = dbamp(32768.0);
	AudioFileDevice *fileDevice = dynamic_cast<AudioFileDevice *>(device);
	
   if (Option::reportClipping() && Option::checkPeaks()) {
      printf("\nPeak amplitudes of output:\n");
      for (n = 0; n < NCHANS; n++) {
	  	if (is_float_format && rtfileit)
			peaks[n] = fileDevice->getPeak(n, &peaklocs[n]);
         double peak_dbfs = dbamp(peaks[n]) - dbref;
         printf("  channel %d: %12.6f (%6.2f dBFS) at frame %ld (%g seconds)\n",
                n, peaks[n], peak_dbfs, peaklocs[n], (float) peaklocs[n] / SR);
      }
   }
}


