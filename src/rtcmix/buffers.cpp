/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Functions to handle allocation of audio buffers and copying between them.
                                                             -JGG, 17-Feb-00
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <globals.h>
#include <assert.h>

/* #define NDEBUG */     /* define to disable asserts */


/* local prototypes */
static BufPtr allocate_buf_ptr(int nsamps);



/* -------------------------------------- copy_interleaved_buf_to_one_buf --- */
/* Copy the specified channel of an interleaved buffer to a one-channel
   buffer. Buffers must be of same type (e.g., float).
*/
#ifdef __GNUC__
inline void
#else
void
#endif
copy_interleaved_buf_to_one_buf(
      BufPtr         dest,            /* buffer containing one chan */
      const BufPtr   src,             /* interleaved buffer */
      int            src_chans,       /* chans in interleaved buffer */
      int            src_chan,        /* channel to copy from */
      int            dest_frames)     /* frames in destination buffer */
{
   int   i, j;

   for (i = 0, j = src_chan; i < dest_frames; i++, j += src_chans)
      dest[i] = src[j];
}


/* -------------------------------------- copy_one_buf_to_interleaved_buf --- */
/* Copy a one-channel buffer into the specified channel of an interleaved
   buffer. Buffers must be of same type (e.g., float).
*/
#ifdef __GNUC__
inline void
#else
void
#endif
copy_one_buf_to_interleaved_buf(
      BufPtr         dest,            /* interleaved buffer */
      const BufPtr   src,             /* buffer containing one chan */
      int            dest_chans,      /* chans in interleaved buffer */
      int            dest_chan,       /* channel to copy into */
      int            dest_frames)     /* frames in destination buffer */
{
   int   i, j;

   for (i = 0, j = dest_chan; i < dest_frames; i++, j += dest_chans)
      dest[j] = src[i];
}


/* ------------------------------------------ copy_interleaved_buf_to_buf --- */
/* Copy one channel between two interleaved buffers. Buffers must be
   of same type (e.g., float).
*/
#ifdef __GNUC__
inline void
#else
void
#endif
copy_interleaved_buf_to_buf(
      BufPtr         dest,            /* interleaved buffer */
      const BufPtr   src,             /* interleaved buffer */
      int            dest_chans,      /* number of chans interleaved */
      int            dest_chan,       /* chan to copy into */
      int            dest_frames,     /* frames in dest buffer */
      int            src_chans,       /* number of chans interleaved */
      int            src_chan)        /* chan to copy from */
{
   int   i, j, k;

   j = src_chan;
   k = dest_chan;
   for (i = 0; i < dest_frames; i++) {
      dest[k] = src[j];
      src_chan += src_chans;
      dest_chan += dest_chans;
   }
}


/* -------------------------------------------------------- init_buf_ptrs --- */
/* Called from main. */
void
init_buf_ptrs()
{
   int   i;

   for (i = 0; i < MAXBUS; i++) {
      audioin_buffer[i] = NULL;
      aux_buffer[i] = NULL;
      out_buffer[i] = NULL;
   }
}


/* ------------------------------------------------ clear_audioin_buffers --- */
// FIXME: not sure we need to do this ever  -JGG
/* Called from inTraverse. */
#ifdef __GNUC__
inline void
#else
void
#endif
clear_audioin_buffers()
{
   int   i, j;

   for (i = 0; i < MAXBUS; i++) {
      BufPtr buf = audioin_buffer[i];
      if (buf != NULL)
         for (j = 0; j < RTBUFSAMPS; j++)
            buf[j] = 0.0;
   }
}


/* ---------------------------------------------------- clear_aux_buffers --- */
/* Called from inTraverse. */
#ifdef __GNUC__
inline void
#else
void
#endif
clear_aux_buffers()
{
   int   i, j;

   for (i = 0; i < MAXBUS; i++) {
      BufPtr buf = aux_buffer[i];
      if (buf != NULL)
         for (j = 0; j < RTBUFSAMPS; j++)
            buf[j] = 0.0;
   }
}


/* ------------------------------------------------- clear_output_buffers --- */
/* Called from inTraverse. */
#ifdef __GNUC__
inline void
#else
void
#endif
clear_output_buffers()
{
   int   i, j;

   for (i = 0; i < NCHANS; i++) {          /* zero just the ones in use */
      BufPtr buf = out_buffer[i];
      for (j = 0; j < RTBUFSAMPS; j++)
         buf[j] = 0.0;
   }
}


/* ----------------------------------------------------- allocate_buf_ptr --- */
#ifdef __GNUC__
static inline BufPtr
#else
static BufPtr
#endif
allocate_buf_ptr(int nsamps)       /* samples, not frames */
{
   BufPtr buf_ptr;

   buf_ptr = (BUFTYPE *) calloc(nsamps, sizeof(BUFTYPE));

   return buf_ptr;
}


/* ---------------------------------------------- allocate_audioin_buffer --- */
// called from ??
#ifdef __GNUC__
inline int
#else
int
#endif
allocate_audioin_buffer(short chan, int nsamps)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (audioin_buffer[chan] == NULL) {
      buf_ptr = allocate_buf_ptr(nsamps);
      assert(buf_ptr != NULL);
      audioin_buffer[chan] = buf_ptr;
   }

   return 0;
}


/* -------------------------------------------------- allocate_aux_buffer --- */
/* Called from bus_config. */
#ifdef __GNUC__
inline int
#else
int
#endif
allocate_aux_buffer(short chan, int nsamps)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (aux_buffer[chan] == NULL) {
      buf_ptr = allocate_buf_ptr(nsamps);
      assert(buf_ptr != NULL);
      aux_buffer[chan] = buf_ptr;
   }

   return 0;
}


/* -------------------------------------------------- allocate_out_buffer --- */
/* Called from rtsetparams. */
#ifdef __GNUC__
inline int
#else
int
#endif
allocate_out_buffer(short chan, int nsamps)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (out_buffer[chan] == NULL) {
      buf_ptr = allocate_buf_ptr(nsamps);
      assert(buf_ptr != NULL);
      out_buffer[chan] = buf_ptr;
   }

   return 0;
}


/* ---------------------------------------------------- allocate_ibuf_ptr --- */
/* Allocate buffer of the type received from the audio input device.
   Called by rtgetsamps.c
*/
#ifdef __GNUC__
inline IBufPtr
#else
IBufPtr
#endif
allocate_ibuf_ptr(int nsamps)       /* samples, not frames */
{
   IBufPtr ibuf_ptr;

   ibuf_ptr = (IBUFTYPE *) malloc(nsamps * sizeof(IBUFTYPE));
   assert(ibuf_ptr != NULL);

   return ibuf_ptr;
}


/* ---------------------------------------------------- allocate_obuf_ptr --- */
/* Allocate buffer of the type sent to the audio output device.
   Called by rtsendsamps.c
*/
#ifdef __GNUC__
inline OBufPtr
#else
OBufPtr
#endif
allocate_obuf_ptr(int nsamps)       /* samples, not frames */
{
   OBufPtr obuf_ptr;

   obuf_ptr = (OBUFTYPE *) malloc(nsamps * sizeof(OBUFTYPE));
   assert(obuf_ptr != NULL);

   return obuf_ptr;
}


