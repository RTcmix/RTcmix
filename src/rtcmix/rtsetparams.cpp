/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* Originally by Brad Garton and Doug Scott (SGI code) and Dave Topper
   (Linux code). Reworked for v2.3 by John Gibson.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <globals.h>
#include <audio_port.h>
#include <sndlibsupport.h>
#include <ugens.h>
#include "../rtstuff/rtdefs.h"

/* #define DEBUG */



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
   int         i, status;
   int         verbose = print_is_on;
#ifdef SGI
   static char *out_port_str = NULL;
#endif /* SGI */

   if (rtsetparams_called) {
      die("rtsetparams", "You can only call rtsetparams once!");
   }

// FIXME: Need better names for NCHANS and RTBUFSAMPS. -JGG

   SR = p[0];
   NCHANS = (int) p[1];
   RTBUFSAMPS = n_args > 2 ? (int) p[2] : 4096;

   if (n_args > 3 && pp[3] != 0.0) {
#ifdef SGI
      /* Cast Minc double to char ptr to get string. */
      int iarg = (int) pp[3];
      out_port_str = (char *) iarg;
 	  advise("rtsetparams", "Playing through output port '%s'", out_port_str);
#endif /* SGI */
   }
   
   if (SR <= 0.0) {
	   die("rtsetparams", "Sampling rate must be greater than 0.");
   }

   if (NCHANS > MAXBUS) {
      die("rtsetparams", "You can only have up to %d output channels.", MAXBUS);
   }

   /* play_audio is true unless user has called set_option("audio_off") before
      rtsetparams. This would let user run multiple jobs, as long as only one
      needs the audio drivers.  -JGG
   */
   if (play_audio) {
      int in_nchans, nframes = RTBUFSAMPS;

      /* Open audio input and output ports. */
#ifdef LINUX
      if (full_duplex)
         in_nchans = NCHANS;
      else
         in_nchans = 0;

      status = open_ports(in_nchans, MAXBUS, in_port, NCHANS, MAXBUS, out_port,
                                         verbose, SR, NUM_FRAGMENTS, &nframes);
#endif /* LINUX */
#ifdef MACOSX
      if (full_duplex)
         in_nchans = NCHANS;
      else
         in_nchans = 0;

      status = open_macosx_ports(in_nchans, NCHANS, &in_port, &out_port,
                                          verbose, SR, NUM_FRAGMENTS, &nframes);
#endif /* MACOSX */
#ifdef SGI
      status = open_sgi_ports(out_port_str, NCHANS, &out_port, verbose, SR,
                                                                     &nframes);
#endif /* SGI */
      if (status == -1) {
         die("rtsetparams", "Trouble opening audio ports.");
      }

      /* This may have been reset by driver. */
      RTBUFSAMPS = nframes;
   }

   /* inTraverse waits for this. Set it even if play_audio is false! */
   pthread_mutex_lock(&audio_config_lock);
   audio_config = 1;
   pthread_mutex_unlock(&audio_config_lock);

   if (verbose)
      printf("Audio set:  %g sampling rate, %d channels\n", SR, NCHANS);

   /* Allocate output buffers. Do this *after* opening audio devices,
      in case OSS changes our buffer size, for example.
   */
   for (i = 0; i < NCHANS; i++) {
      allocate_out_buffer(i, RTBUFSAMPS);
   }

   rtsetparams_called = 1;	/* Put this at end to allow re-call due to error */

   return 0;
}


/* ---------------------------------------------------- close_audio_ports --- */
/* NOTE: This is here, rather than in audio_port.c, because it needs globals
   that we'd prefer to hide from utility programs (like cmixplay.c) that
   use audio_port.c.    -JGG
*/
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

#ifdef MACOSX
// FIXME: not sure what to do yet...
   if (out_port) ;
   if (in_port) ;
#endif /* MACOSX */

#ifdef SGI
   if (out_port) {
      while (ALgetfilled(out_port) > 0)
         ;
// FIXME: anything else?
   }
#endif /* SGI */
}

