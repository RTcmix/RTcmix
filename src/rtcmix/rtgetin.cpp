/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* Functions to fill instrument input buffers from file, aux bus or audio dev.
                                                             -JGG, 17-Feb-00
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <globals.h>
#include <sndlibsupport.h>
#include <byte_routines.h>
#include <Instrument.h>
#include <rtdefs.h>
#include <assert.h>


// FIXME: Need to clean up return value confusion below. Should return val
//        indicate status, nframes, nsamps, nbytes?
//        (Was nbytes, but always ignored.)



/* ----------------------------------------------------------- get_aux_in --- */
static int
get_aux_in(
      BufPtr   dest,             /* interleaved buffer from inst */
      int      dest_chans,       /* number of chans interleaved */
      int      dest_frames,      /* frames in interleaved buffer */
      short    src_chan_list[],  /* list of auxin-bus chan numbers from inst */
      short    src_chans)        /* number of auxin-bus chans to copy */
{
   assert(dest_chans >= src_chans);

   for (int n = 0; n < src_chans; n++) {
      int chan = src_chan_list[n];

      BufPtr src = aux_buffer[chan];
      assert(src != NULL);

      copy_one_buf_to_interleaved_buf(dest, src, dest_chans, n, dest_frames);
   }

   return 0;
}


/* --------------------------------------------------------- get_audio_in --- */
static int
get_audio_in(
      BufPtr   dest,             /* interleaved buffer from inst */
      int      dest_chans,       /* number of chans interleaved */
      int      dest_frames,      /* frames in interleaved buffer */
      short    src_chan_list[],  /* list of in-bus chan numbers from inst */
      short    src_chans)        /* number of in-bus chans to copy */
{
   int   audioin_chans = 2;  // FIXME: where do we get this? rtinput pfield

   assert(dest_chans >= src_chans);
   assert(audioin_chans >= src_chans);

   for (int n = 0; n < src_chans; n++) {
      int chan = src_chan_list[n];

      BufPtr src = audioin_buffer[chan];
      assert(src != NULL);

      copy_one_buf_to_interleaved_buf(dest, src, dest_chans, n, dest_frames);
   }

   return 0;
}


/* ----------------------------------------------------- read_float_samps --- */
static int
read_float_samps(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      Instrument  *inst)
{
   int            fd, file_chans, seeked, nsamps, src_samps, swap;
   int            data_format, datum_size;
   ssize_t        bytes_to_read, bytes_read;
   char           *bufp;
   static float   *fbuf = NULL;
   
   /* File opened by earlier call to rtinput. */
   fd = inputFileTable[inst->fdIndex].fd;
   file_chans = inputFileTable[inst->fdIndex].chans;

   assert(file_chans >= dest_chans);

   data_format = inputFileTable[inst->fdIndex].data_format;

#ifdef SNDLIB_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif

   if (fbuf == NULL) {     /* 1st time, so allocate interleaved float buffer */
      fbuf = (float *) malloc((size_t) RTBUFSAMPS * MAXCHANS);
      if (fbuf == NULL) {
         perror("read_float_samps (malloc)");
         exit(1);
      }
   }
   bufp = (char *) fbuf;

// FIXME: Needs fix to rtsetinput when setting initial fileOffset for inst!

   if (lseek(fd, inst->fileOffset, SEEK_SET) == -1) {
      perror("read_float_samps (lseek)");
      exit(1);
   }

   datum_size = sizeof(float);

   bytes_to_read = dest_frames * file_chans * datum_size;

   while (bytes_to_read > 0) {
      bytes_read = read(fd, bufp, bytes_to_read);
      if (bytes_read == -1) {
         perror("read_float_samps (read)");
         exit(1);
      }
      if (bytes_read == 0)          /* EOF */
         break;

      bufp += bytes_read;
      bytes_to_read -= bytes_read;
   }

   /* If we reached EOF, zero out remaining part of buffer that we
      expected to fill.
   */
   while (bytes_to_read > 0) {
      (* (float *) bufp) = 0.0;
      bufp += datum_size;
      bytes_to_read -= datum_size;
   }

   /* Copy interleaved input buffer to inst's input buffer, with bus mapping. */

   src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
      int chan = src_chan_list[n];
      int j = n;
      for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
         dest[j] = (BUFTYPE) fbuf[i];
   }

   if (swap) {
      for (int i = 0; i < src_samps; i++)
         byte_reverse4(&dest[i]);
   }

   /* NOTE: Just return size of entire buffer, even if we had to do
            some zero padding.
   */
   nsamps = dest_frames / dest_chans;

   return nsamps;
}


/* ---------------------------------------------------- sndlib_read_samps --- */
static int
sndlib_read_samps(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      Instrument  *inst)
{
   int         fd, file_chans, seeked, nsamps;
   static int  **inbufs = NULL;

   /* File opened by earlier call to rtinput. */
   fd = inputFileTable[inst->fdIndex].fd;
   file_chans = inputFileTable[inst->fdIndex].chans;

   assert(file_chans >= dest_chans);

   if (inbufs == NULL) {    /* 1st time, so allocate array of buffers */
      inbufs = sndlib_allocate_buffers(MAXCHANS, RTBUFSAMPS);
      if (inbufs == NULL) {
         perror("sndlib_read_samps: Can't allocate input buffers");
         exit(1);
      }
   }

   /* Go to saved file offset for this instrument.
      NOTE: clm_seek handles any datum size as if it were 16 bits.
   */
   seeked = clm_seek(fd, inst->fileOffset, SEEK_SET);
   if (seeked == -1) {
      fprintf(stderr, "Bad seek on the input soundfile\n");
      exit(1);
   }

// FIXME: doesn't return an error code! Upgrade to new sndlib for that.
   clm_read(fd, 0, dest_frames-1, file_chans, inbufs);

   for (int n = 0; n < dest_chans; n++) {
      int chan = src_chan_list[n];
      int j = n;
      for (int i = 0; i < dest_frames; i++, j += dest_chans)
         dest[j] = (BUFTYPE) inbufs[chan][i];
   }

   /* NOTE: We can't know how much zero padding sndlib did, so just use
            dest_frames.
   */
   nsamps = dest_frames / dest_chans;

   return nsamps;
}


/* ---------------------------------------------------------- get_file_in --- */
static int
get_file_in(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      Instrument  *inst)
{
   int   data_format, nsamps;

   assert(dest_chans >= src_chans);

   data_format = inputFileTable[inst->fdIndex].data_format;

   /* We read float files ourselves, rather than hand them to sndlib. */
   if (IS_FLOAT_FORMAT(data_format)) {
      nsamps = read_float_samps(dest, dest_chans, dest_frames, src_chan_list,
                                                             src_chans, inst);
      inst->fileOffset += nsamps * sizeof(float);
   }
   else {
      nsamps = sndlib_read_samps(dest, dest_chans, dest_frames, src_chan_list,
                                                             src_chans, inst);
      /* NOTE: This is right for sndlib, even if file isn't 16-bit: */
      inst->fileOffset += nsamps * sizeof(short);
   }

   return nsamps;
}


/* -------------------------------------------------------------- rtgetin --- */
// some limitations for now...

/* For use by instruments that take input either from an in buffer or from
   an aux buffer, but not from both at once. Also, input from files or
   from the audio in device can come only through an in buffer, not from
   an aux buffer.
*/
int
rtgetin(float      *inarr,         /* interleaved array of <inputchans> */
        Instrument *inst,
        int        nsamps)         /* samps, not frames */
{
   int   frames, status, fdindex;
   int   inchans = inst->inputchans;    /* total in chans inst expects */
   short in_count = inst->bus_config->in_count;
   short auxin_count = inst->bus_config->auxin_count;

   assert(inarr != NULL);

   fdindex = inst->fdIndex;

   frames = nsamps / inchans;

   if (frames > RTBUFSAMPS) {
      fprintf(stderr, "Internal Error: rtgetin: nsamps out of range!\n");
      exit(1);
   }

   if (fdindex == NO_DEVICE_FDINDEX) {               /* input from aux buses */
      short *auxin = inst->bus_config->auxin;        /* auxin channel list */

      assert(auxin_count > 0 && in_count == 0);

      status = get_aux_in(inarr, inchans, frames, auxin, auxin_count);
   }
   else if (inputFileTable[fdindex].is_audio_dev) {  /* input from mic/line */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_audio_in(inarr, inchans, frames, in, in_count);
   }
   else {                                            /* input from file */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_file_in(inarr, inchans, frames, in, in_count, inst);
   }

   return status;  // FIXME: or was this supposed to be nsamps or nbytes?
}



/*

// How about an alternative function for insts that want to be more bus savvy?

int rtgetin_from_bus(float inarr[], Instrument *inst, int nsamps,
                     BusType bus_type, int bus_chan)

// Problem #1: what buses get input from fdIndex?
// Need more than one fdIndex per inst (for convolve, e.g.)?

// Hook up with idea for:  rtinput("foo.aiff", "aux 1-2 in")

// Would need many changes all over this file (and others).

*/
