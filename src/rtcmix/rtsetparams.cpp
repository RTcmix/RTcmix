/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* Originally by Brad Garton and Doug Scott (SGI code) and Dave Topper
   (Linux code). Reworked for v2.3 by John Gibson.
*/
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include "../rtstuff/rtdefs.h"

#ifdef LINUX
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <sys/ioctl.h>
   #ifdef ALSA
      #include <sys/soundcard.h>
      #define open_ports()  open_alsa_ports()
   #endif
   #ifdef OSS
      #include <sys/soundcard.h>
      #define open_ports()  open_oss_ports()
   #endif

   #define AUDIO_DATUM_SIZE   (sizeof(short))
   #define BASE_SND_DEVICE    "/dev/dsp"
   #define NUM_OSS_FRAGMENTS   10
#endif /* LINUX */

#ifdef SGI
   #include <dmedia/audio.h>
   #include <ulocks.h>

  /* This macro is only in AL 2.0, so I use it to distinguish 2.0 from 1.0 */
   #ifdef AL_MAX_STRLEN
      #define AL_2_0
   #endif

   #ifndef AL_2_0
      #define alSetSampFmt ALsetsampfmt
      #define alSetFloatMax ALsetfloatmax
      #define alSetWidth ALsetwidth
      #define alSetChannels ALsetchannels
      #define alOpenPort ALopenport
      #define alClosePort ALcloseport
      #define alSetFloatMax ALsetfloatmax
      #define alNewConfig ALnewconfig
      #define alFreeConfig ALfreeconfig
      #define alSetConfig ALsetconfig
      #define alSetQueueSize ALsetqueuesize

      /* this function doesn't exist under 1.0 */
      const char *alGetErrorString(int err) { return ""; }
   #endif /* !AL_2_0 */
   static char *output_port = NULL;
   #ifdef NOTYET
      extern usema_t *audio_sema;      /* in Minc/intraverse.C */
   #endif
#endif /* SGI */


#ifdef LINUX

/* ---------------------------------------------------------- open_device --- */
/* Open the specified audio device, and return its file descriptor.
   If error on open, print error msg and return -1.

   <num> is a number appended to the BASE_SND_DEVICE string.
         If -1, then no number appended.

   <access_type> are the flags passed to open (e.g., O_WRONLY).

   Examples (if BASE_SND_DEVICE is "/dev/dsp"):
      open_device(2, O_RDWR);      ... opens "/dev/dsp2" for read/write access.
      open_device(-1, O_RDONLY);   ... opens "/dev/dsp" for read-only access.
*/
static int
open_device(int num, int access_type)
{
   int   fd;
   char  buf[256];

   if (num == -1)
      strcpy(buf, BASE_SND_DEVICE);
   else
      sprintf(buf, "%s%d", BASE_SND_DEVICE, num);

   fd = open(buf, access_type, 0);
   if (fd == -1) {
      char  str[256];
      sprintf(str, "open_device: (%s)", buf);
      perror(str);
   }

   return fd;
}


#ifdef ALSA

/* ------------------------------------------------------ open_alsa_ports --- */
static int
open_alsa_ports()
{
// FIXME: needs much work  -JGG

   if (out_port) {
      fprintf(stderr, "Audio ports already opened!\n");
      return -1;
   }

#ifdef MONO_DEVICES
#else /* !MONO_DEVICES */
#endif /* !MONO_DEVICES */

   return 0;
err:
   fprintf(stderr, "Could not open audio ports.\n");
   return -1;
}

#endif /* ALSA */


#ifdef OSS

/* ----------------------------------------------------- setup_oss_device --- */
/* Set up the audio device parameters. Return 0 if ok, -1 if error.
*/
static int
setup_oss_device(
      int   fd,             /* file descriptor of device just opened */
      int   nchans,         /* number of chans for this device */
      int   srate,          /* sampling rate */
      int   num_frags,      /* number of buffer fragments */
      int   full_duplex,    /* 1 if you want a full duplex device */
      int   *buffer_size)   /* desired buffer size; actual size returned */
{
   int      format, arg, stereo, tmp, frag_size;
   double   bufexponent;

   assert(fd >= 0);

   // FIXME: Is this really supposed to go before the others?  -JG
   if (full_duplex) {
      if (ioctl(fd, SNDCTL_DSP_SETDUPLEX, &tmp) == -1) {
         perror("SNDCTL_DSP_SETDUPLEX");
         return -1;
      }
   }

   /* Set buffering parameters (number of fragments and fragment size).
      OSS requires this way of specifying things.
   */
   arg = num_frags << 16;
   bufexponent = log((double) *buffer_size) / log(2.0);
   arg += (int) bufexponent;
#ifdef DEBUG
   printf("arg: %#10.8x\n", arg);
#endif
   if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &arg) == -1) {
      perror("SNDCTL_DSP_SETFRAGMENT");
      return -1;
   }

   /* Set sample format */
#ifdef SNDLIB_LITTLE_ENDIAN
   format = AFMT_S16_LE;
#else
   format = AFMT_S16_BE;
#endif
   if (ioctl(fd, SNDCTL_DSP_SETFMT, &format) == -1) {
      perror("SNDCTL_DSP_SETFMT");
      return -1;
   }

   /* Set stereo / mono (1/0) */
   stereo = nchans - 1;
   if (ioctl(fd, SNDCTL_DSP_STEREO, &stereo) == -1) {
      perror("SNDCTL_DSP_STEREO");
      return -1;
   }

   /* Set sample rate */
   if (ioctl(fd, SNDCTL_DSP_SPEED, &srate) == -1) {
      perror("SNDCTL_DSP_SPEED");
      return -1;
   }

   /* Get size of audio buffer. Note that driver computes this optimally. */
   if (ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &frag_size) == -1) {
      perror("SNDCTL_DSP_GETBLKSIZE");
      return -1;
   }
#ifdef DEBUG
   printf("Buffer size: %d (requested), %d (actual)\n",
                                                    *buffer_size, frag_size);
#endif

   return 0;
}


/* ------------------------------------------------------- open_oss_ports --- */
static int
open_oss_ports()
{
   int   n, status, datum_size, buf_bytes;

   datum_size = AUDIO_DATUM_SIZE;

#ifdef MONO_DEVICES

   buf_bytes = RTBUFSAMPS * sizeof(short);

   for (n = 0; n < NCHANS; n++) {
      out_port[n] = open_device(n, O_WRONLY);
      if (out_port[n] == -1)
         goto err;
      status = setup_oss_device(out_port[n], 1, (int) SR,
                                          NUM_OSS_FRAGMENTS, 0, &buf_bytes);
   }

   if (full_duplex) {
//FIXME: But it's NOT necessarily NCHANS!
      for (n = 0; n < NCHANS; n++) {
         in_port[n] = open_device(n, O_RDONLY);
         if (in_port[n] == -1)
            goto err;
         status = setup_oss_device(in_port[n], 1, (int) SR,
                                          NUM_OSS_FRAGMENTS, 0, &buf_bytes);
      }
   }

#else /* !MONO_DEVICES */

   if (out_port[0]) {
      fprintf(stderr, "Audio ports already opened!\n");
      return -1;
   }

   /* This works for Creative/Ensoniq AudioPCI and other consumer cards. */
   if (full_duplex) {
      out_port[0] = open_device(-1, O_RDWR);
      if (out_port[0] == -1)
         goto err;
      in_port[0] = out_port[0];
   }
   else {
      out_port[0] = open_device(-1, O_WRONLY);
      if (out_port[0] == -1)
         goto err;
   }
   buf_bytes = RTBUFSAMPS * NCHANS * sizeof(short);
   status = setup_oss_device(out_port[0], NCHANS, (int) SR,
                                 NUM_OSS_FRAGMENTS, full_duplex, &buf_bytes);

#endif /* !MONO_DEVICES */

   printf("Output buffer:  %d bytes\n", buf_bytes);
   if (full_duplex)
      printf("Input buffer:   %d bytes\n", buf_bytes);

   /* I think OSS can change our requested buffer size.  -JGG */
   RTBUFSAMPS = (buf_bytes / datum_size) / NCHANS;

   return status;
err:
   fprintf(stderr, "Could not open audio ports.\n");
   return -1;
}
#endif /* OSS */

#endif /* LINUX */


#ifdef SGI

/* ------------------------------------------------------- open_sgi_ports --- */
/* Configure and open output audio port.
   We will write floats directly to the device, letting it know that they
   will be within short integer range so we do not need to limit.  This
   reduces the CPU overhead of the alWriteSamps call.
*/
// FIXME: We don't yet support this float-writing in the
//        Linux-derived SGI port.   -JGG
static int
open_sgi_ports()
{
   long     pvbuf[2], buflen;
   ALconfig out_port_config;

   out_port_config = alNewConfig();

#ifdef AL_2_0
   /* Set output port if we specified one. */
   if (output_port != NULL) {
      int device = alGetResourceByName(AL_SYSTEM, output_port, AL_DEVICE_TYPE);
      if (device > 0)
         alSetDevice(out_port_config, device);
      else {
         fprintf(stderr, "WARNING: The audio port you specified: '%s', "
                         "is invalid\nUsing default port.\n", output_port);
         alSetDevice(out_port_config, AL_DEFAULT_DEVICE);
      }
   }
   alSetLimiting(out_port_config, 0);
#endif

#ifdef SEND_FLOATS_TO_DAC
   alSetSampFmt(out_port_config, AL_SAMPFMT_FLOAT);
   alSetFloatMax(out_port_config, 32768.0);
#else
   alSetSampFmt(out_port_config, AL_SAMPFMT_TWOSCOMP);
   alSetWidth(out_port_config, AL_SAMPLE_16);
#endif

   alSetChannels(out_port_config, NCHANS);

   if (alSetQueueSize(out_port_config, RTBUFSAMPS * 4) == -1) {
      fprintf(stderr, "Could not configure the output audio port queue "
                      "size to %d\n", RTBUFSAMPS * 4);
      alFreeConfig(out_port_config);
      return -1;
   }
   pvbuf[0] = AL_OUTPUT_RATE;
   pvbuf[1] = (long) SR;
   buflen = 2;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

// FIXME: We can't handle buffer size changing yet.   -JGG
#ifdef NOT_YET
   /* open up thet thar audio port! (if it has not been opened).
      otherwise, attempt to reconfigure the existing port.  If we have
      changed buffer size, we need to close and reopen.
   */
   if (out_port != NULL && newBufSamps != RTBUFSAMPS) {
      if (alClosePort(out_port) < 0) {
         fprintf(stderr,
                 "Could not close output audio port for reconfigure:  %s\n",
                 alGetErrorString(oserror()));
         return -1;
      }
      out_port = NULL;
      RTBUFSAMPS = newBufSamps;
   }
#endif /* NOT_YET */
   if (out_port == NULL)
      out_port = alOpenPort("rtcmixout", "w", out_port_config);
   else if (alSetConfig(out_port, out_port_config) < 0) {
      fprintf(stderr, "Could not reconfigure output audio port:  %s\n",
              alGetErrorString(oserror()));
   }

   alFreeConfig(out_port_config);

   if (!out_port) {
      fprintf(stderr, "Could not open output audio port:  %s\n",
              alGetErrorString(oserror()));
      exit(-1);
   }

   noaudio = 1;
#ifdef NOTYET
   if (audio_sema)
      usvsema(audio_sema);         /* wake up server if in that mode */
#endif

   return 0;
}

#endif /* SGI */


/* ---------------------------------------------------------- rtsetparams --- */
/* Minc function that sets output sampling rate (p0), maximum number of
   output channels (p1), and (optionally) I/O buffer size (p2).

   On SGI, the optional p3 lets you specify the output port as a string
   (e.g., "LINE", "DIGITAL", etc.).

   Based on this information, rtsetparams allocates a mono output buffer
   for each of the output channels, and opens output devices.
*/
double
rtsetparams(float p[], int n_args, double pp[])
{
   int   i, status;

// FIXME: Need better names for NCHANS, RTBUFSAMPS and MAXBUF. Would prefer
//        to get rid of MAXBUF or make it a constant again.  -JGG

   SR = p[0];
   NCHANS = (int) p[1];
   RTBUFSAMPS = n_args > 2 ? (int) p[2] : 8192;

   if (n_args > 3) {
#ifdef SGI
      /* Cast Minc double to char ptr to get string. */
      int iarg = (int) pp[3];
      output_port = (char *) iarg;
      printf("Playing through output port '%s'\n", output_port);
#endif /* SGI */
   }

   if (NCHANS > MAXBUS) {
      fprintf(stderr, "You can only have %d output channels.\n", MAXBUS);
      exit(1);
   }

   /* play_audio is true unless user has called set_option("audio_off") before
      rtsetparams. This would let user run multiple jobs, as long as only one
      needs the audio drivers.  -JGG
   */
   if (play_audio) {
      /* Open audio input and output ports. */
#ifdef LINUX
      status = open_ports();
#endif /* LINUX */
#ifdef SGI
      status = open_sgi_ports();
#endif /* SGI */
      if (status == -1)
         exit(1);
   }

   /* inTraverse waits for this. Set it even if play_audio is false! */
   audio_config = 1;

   printf("Audio set:  %g sampling rate, %d channels\n", SR, NCHANS);

   /* Allocate output buffers. Do this *after* opening audio devices,
      in case OSS changes our buffer size, for example.
   */
   for (i = 0; i < NCHANS; i++) {
      allocate_out_buffer(i, RTBUFSAMPS);
   }

   MAXBUF = (int) NCHANS * RTBUFSAMPS;

   rtsetparams_called = 1;

   return 0.0;
}


/* ---------------------------------------------------- close_audio_ports --- */
void
close_audio_ports()
{
   int n;

#ifdef LINUX
 #ifdef MONO_DEVICES
   for (n = 0; n < NCHANS; n++)
      if (out_port[n])
         close(out_port[n]);
   for (n = 0; n < audioNCHANS; n++)
      if (in_port[n])
         close(in_port[n]);
 #else /* !MONO_DEVICES */
   if (out_port[0])
      close(out_port[0]);
   if (in_port[0] && in_port[0] != out_port[0])
      close(in_port[0]);
 #endif /* !MONO_DEVICES */
#endif /* LINUX */

#ifdef SGI
   if (out_port) {
      while (ALgetfilled(out_port) > 0)
         ;
// FIXME: anything else?
   }
#endif /* SGI */
}

