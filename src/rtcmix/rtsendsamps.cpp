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
#include "../rtstuff/rtdefs.h"
#include "AudioDevice.h"
#include "AudioFileDevice.h"

/* #define DUMP_AUDIO_TO_RAW_FILE */
/* #define USE_REAL2INT */

extern AudioDevice *globalOutputFileDevice;	// audio_devices.cpp

static int printing_dots = 0;
static int dev_obuf_allocated = 0;

/* Array of output buffers for audio device, in format wanted by driver.
   If driver wants interleaved samps, use only dev_obuf[0], which contains
   interleaved samples. Otherwise, each buffer in dev_obuf contains one
   channel of samples.
*/
static OBufPtr dev_obuf[MAXBUS];

/* local prototypes */
static void allocate_dev_obuf(void);
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


/* ---------------------------------------------------- allocate_dev_obuf --- */
/* Allocate either one interleaved buffer or multiple single buffers,
   depending on whether MONO_DEVICES is defined.
*/
static void
allocate_dev_obuf()
{
#ifdef MONO_DEVICES
   int i;

   for (i = 0; i < NCHANS; i++)
      dev_obuf[i] = allocate_obuf_ptr(RTBUFSAMPS);
#else /* !MONO_DEVICES */
   dev_obuf[0] = allocate_obuf_ptr(RTBUFSAMPS * NCHANS);
#endif /* !MONO_DEVICES */
}


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
   int   i, j, n, result;

#ifdef MONO_DEVICES
   const int nbytes = RTBUFSAMPS * sizeof(OBUFTYPE);

   for (n = 0; n < NCHANS; n++) {
      BufPtr src = out_buffer[n];
      OBufPtr dest = dev_obuf[n];

      for (i = 0; i < RTBUFSAMPS; i++)
         dest[i] = (OBUFTYPE) src[i];

      result = write(out_port[n], dest, nbytes);
      if (result == -1)
         break;
   }

#else /* !MONO_DEVICES */

   OBufPtr dest = dev_obuf[0];

   for (n = 0; n < NCHANS; n++) {
      BufPtr src = out_buffer[n];

      for (i = 0, j = n; i < RTBUFSAMPS; i++, j += NCHANS)
#ifdef USE_REAL2INT
         dest[j] = real2int(src[i]);
#else
         dest[j] = (OBUFTYPE) src[i];
#endif
   }
#ifdef DUMP_AUDIO_TO_RAW_FILE
   dump_audio_to_raw_file(dest, nbytes);
#endif
	result = device->sendFrames(dest, RTBUFSAMPS);

#endif /* !MONO_DEVICES */

   if (result == -1)
      return -1;
   return 0;
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
         if (check_peaks) {
            BUFTYPE abs_samp = (BUFTYPE) fabs((double) buf[j]);
            if (abs_samp > peaks[i]) {
               peaks[i] = abs_samp;
               peaklocs[i] = bufStartSamp + (j / NCHANS);  /* frame count */
            }
         }
      }
   }

   if (numclipped && report_clipping) {
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

   if (play_audio) {
      int i, j, nsamps, nbufs;

      if (!dev_obuf_allocated)
         allocate_dev_obuf();

      err = write_to_audio_device(device);
      if (err)
         fprintf(stderr, "rtsendzeros: bad write to audio device\n");
   }

   if (also_write_to_file && rtfileit) {
      err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendzeros: bad write to output sound file\n");
   }
}


/* ---------------------------------------------------------- rtsendsamps --- */
/* Called by the scheduler to write the output buffer to the audio device
   and/or a sound file. This involves two kinds of conversion:

      1) Convert the sample format from the type used in the internal
         output buffers (floating point) to that expected by the audio
         device (if playing) and the output file (if writing). Most
         audio devices expect 16-bit or 24-bit integers, but some
         (like SGI) can accept 32-bit floats efficiently. Sound files
         can be most any kind of format. We need to support 16-bit and
         24-bit ints, as well as 32-bit floats, optionally normalized
         to fit mostly into the range [-1.0, 1.0].

         Converting from floats to ints involves detecting, reporting
         and handling out-of-range samples.

      2) As required, convert the buffer scheme from one buffer per
         channel to one big buffer with channels interleaved. All sound
         files, and some audio devices, need interleaved samples.
*/

#define NPLAY_BUFSIZE 8192

void
rtsendsamps(AudioDevice *device)
{
   int   err;

   /* If we're writing to a file, and not playing, print a dot to show
      we've output one buffer.
   */
   if (!play_audio && rtfileit) {
      printing_dots = 1;
      printf(".");    /* no '\n' */
   }

   /* Write float file *before* doing any limiting. */
   if (is_float_format && rtfileit) {
	  // FOR NOW, IF WE ARE BOTH PLAYING AND WRITING, DO IT WITH SEPARATE
	  // AudioDevice INSTANCES.
	  if (play_audio)
      	err = rtwritesamps(globalOutputFileDevice);
	  else
	  	err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendsamps: bad write to output sound file\n");
      if (!play_audio)
         return;        /* without limiting */
   }

#ifdef LIMIT_OBUF_FOR_AUDIO_DEV
   limiter();    /* If audio device does *not* want floats, do limiting now. */
#endif

   if (play_audio) {
      if (!dev_obuf_allocated)
         allocate_dev_obuf();

      err = write_to_audio_device(device);
      if (err)
         fprintf(stderr, "rtsendsamps: bad write to audio device\n");
   }

   if (!is_float_format && rtfileit) {
#ifndef LIMIT_OBUF_FOR_AUDIO_DEV
      limiter();                        /* Do limiting just for output file. */
#endif
	  // FOR NOW, IF WE ARE BOTH PLAYING AND WRITING, DO IT WITH SEPARATE
	  // AudioDevice INSTANCES.
	  if (play_audio)
      	err = rtwritesamps(globalOutputFileDevice);
	  else
	  	err = rtwritesamps(device);
      if (err)
         fprintf(stderr, "rtsendsamps: bad write to output sound file\n");
   }

#ifdef NETPLAYER
   if (netplay == 1) {        /* send to the socket connection */
      int   i, j, result;
      short outbufshort[NPLAY_BUFSIZE];

      if (!dev_obuf_allocated)
         allocate_dev_obuf();
      for (i = 0; i < NPLAY_BUFSIZE / 2; i++) {
         for (j = 0; j < 2; j++)
            outbufshort[i * 2 + j] = out_buffer[j][i];
      }
      result = write(netplaysock, &outbufshort, RTBUFSAMPS * NCHANS
                                                            * sizeof(short));
      if (result < 0) {
         fprintf(stderr, "bad write to network sound socket\n");
         return;
      }
   }
#endif
}


/* -------------------------------------------------------- rtreportstats --- */
void
rtreportstats(AudioDevice *device)
{
	int    n;
	double dbref = dbamp(32768.0);
	AudioFileDevice *fileDevice = dynamic_cast<AudioFileDevice *>(device);
	
   if (report_clipping && check_peaks) {
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


