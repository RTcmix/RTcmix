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

/* The define below is to disable some fancy bus-mapping code for file
   input that was not well thought out. As a user, when I open a file,
   I typically just want the instrument to read all its channels without
   me worrying how many there are. If the instrument can read only one,
   I'll tell it which one in its inchan pfield. But what numbers do I use
   in my call to bus_config for the "in" bus? As a user, I don't care.
   So we're ignoring them here, and making sure that inst->inputchans
   is set to match the number of chans in the file (in rtsetinput).

   What we lose by ignoring the bus numbers is consistency of the
   bus_config interface between file and rt input. We also lose the
   ability to select arbitrary channels from a file to send into an
   instrument. (But what instruments could make use of this?) My
   feeling is that this selection feature would be more trouble for
   the user than it's worth. What do you think?

   If we expand the rtinput syntax to allow opening more than one
   file for an instrument to use, then this could all change. But
   it's not easy to see how, exactly. Note that the commented out
   code below interprets the bus numbers as a selection of channels
   from the input file. However, if you could say:

      rtinput("foo.snd", "in 0-1"); rtinput("bar.snd", "in 2-3")

   Then you'd expect this to work the way an aux-in bus does now: the
   first file is read into buses 0-1, the second into buses 2-3. But
   these "buses" aren't really buses the way aux buses are. Instead,
   they're just private buffers in this file (or in an instrument,
   depending on how you look at it). This makes sense, because sound
   files aren't real-time input, and instruments often don't read from
   the same section of a file during the same timeslice -- so they'd
   hardly ever be able to share a file input bus. So with the rtinput
   syntax above, we'd just pretend that those are real buses. What
   kind of bus_config would work with those two rtinputs, and what
   would its in-bus numbers mean?

   JGG, 27-May-00
*/
#define IGNORE_BUS_COUNT_FOR_FILE_INPUT



/* ------------------------------------------------------------ rtinrepos --- */
/* Change rt input sound file position pointer, in preparation for a
   subsequent call to rtgetin.  Designed for use by instruments that 
   want to reposition the input file arbitrarily (like REVMIX, which
   reads a file backwards).

   <whence> works just as in lseek: it specifies the point from which
   to move <frames> number of frames.  <frames> can be negative.
   Values of <whence> are SEEK_SET, SEEK_CUR and SEEK_END, all defined
   in <unistd.h>.

   This function doesn't actually do an lseek on the file yet -- that
   happens inside of rtgetin -- this merely updates the instrument's
   <fileOffset> for the next read.
*/
int
rtinrepos(Instrument *inst, int frames, int whence)
{
   int   fdindex, fd, datum_size;
   off_t bytes;

   fdindex = inst->fdIndex;

   if (fdindex == NO_DEVICE_FDINDEX || inputFileTable[fdindex].is_audio_dev) {
      fprintf(stdin,
            "rtinrepos: request to reposition input, but input is not a file.");
   }

   fd = inputFileTable[fdindex].fd;

   if (inputFileTable[fdindex].is_float_format)
      datum_size = sizeof(float);
   else
      datum_size = 2;

   bytes = frames * inst->inputchans * datum_size;

   switch (whence) {
      case SEEK_SET:
         assert(bytes >= 0);
         inst->fileOffset = inputFileTable[fdindex].data_location + bytes;
         break;
      case SEEK_CUR:
         inst->fileOffset += bytes;
         break;
      case SEEK_END:
         fprintf(stderr, "rtinrepos: SEEK_END unimplemented\n");
         exit(1);
         break;
      default:
         fprintf(stderr, "Instrument error: rtinrepos: invalid <whence>\n");
         exit(1);
         break;
   }

   return 0;
}


/* ----------------------------------------------------------- get_aux_in --- */
static int
get_aux_in(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of auxin-bus chan nums from inst */
      short       src_chans,        /* number of auxin-bus chans to copy */
      Instrument  *inst)
{
   assert(dest_chans >= src_chans);

   for (int n = 0; n < src_chans; n++) {
      int chan = src_chan_list[n];

      BufPtr src = aux_buffer[chan];
      assert(src != NULL);

      /* The inst might be playing only the last part of a buffer. If so,
         we want it to read the corresponding segment of the aux buffer.
      */
      src += inst->output_offset;

      copy_one_buf_to_interleaved_buf(dest, src, dest_chans, n, dest_frames);
   }

   return 0;
}


/* --------------------------------------------------------- get_audio_in --- */
static int
get_audio_in(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      Instrument  *inst)
{
#ifdef NOMORE  // FIXME: not sure these are right
   assert(dest_chans >= src_chans);
   assert(audioNCHANS >= src_chans);
#endif

   for (int n = 0; n < src_chans; n++) {
      int chan = src_chan_list[n];

      BufPtr src = audioin_buffer[chan];
      assert(src != NULL);

      /* The inst might be playing only the last part of a buffer. If so,
         we want it to read the corresponding segment of the audioin buffer.
      */
      src += inst->output_offset;

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
   int            fd, file_chans, seeked, src_samps, swap;
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
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      int chan = n;
#else
      int chan = src_chan_list[n];
#endif
      int j = n;
      for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
         dest[j] = (BUFTYPE) fbuf[i];
   }

   if (swap) {
      for (int i = 0; i < src_samps; i++)
         byte_reverse4(&dest[i]);
   }

   /* Advance saved offset by the number of samps (not frames) read.
      Note that this includes samples in channels that were read but
      not copied into the dest buffer!
   */
   inst->fileOffset += dest_frames * file_chans * sizeof(float);

   return 0;
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
   int         fd, file_chans, seeked;
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
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      int chan = n;
#else
      int chan = src_chan_list[n];
#endif
      int j = n;
      for (int i = 0; i < dest_frames; i++, j += dest_chans)
         dest[j] = (BUFTYPE) inbufs[chan][i];
   }

   /* Advance saved offset by the number of samps (not frames) read.
      Note that this includes samples in channels that were read but
      not copied into the dest buffer! Note also that sndlib wants
      offsets as if the file were 16 bits (even when it's not).
   */
   inst->fileOffset += dest_frames * file_chans * 2;

   return 0;
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
   int status;

#ifndef IGNORE_BUS_COUNT_FOR_FILE_INPUT
   assert(dest_chans >= src_chans);
#endif

   /* We read float files ourselves, rather than hand them to sndlib. */
   if (inputFileTable[inst->fdIndex].is_float_format) {
      status = read_float_samps(dest, dest_chans, dest_frames, src_chan_list,
                                                             src_chans, inst);
   }
   else {
      status = sndlib_read_samps(dest, dest_chans, dest_frames, src_chan_list,
                                                             src_chans, inst);
   }

   return status;
}


/* -------------------------------------------------------------- rtgetin --- */
/* For use by instruments that take input either from an in buffer or from
   an aux buffer, but not from both at once. Also, input from files or from
   the audio in device can come only through an in buffer, not from an aux
   buffer. Note that file input ignores the bus_config entirely, simply
   reading all the file channels into the insts buffer, which we assume is
   large enough to hold them. (This should be a safe assumption, since
   rtsetinput makes sure that inst->inputchans matches the file chans.)
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

      status = get_aux_in(inarr, inchans, frames, auxin, auxin_count, inst);
   }
   else if (inputFileTable[fdindex].is_audio_dev) {  /* input from mic/line */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_audio_in(inarr, inchans, frames, in, in_count, inst);
   }
   else {                                            /* input from file */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_file_in(inarr, inchans, frames, in, in_count, inst);
   }

   return nsamps;   // this seems pointless, but no insts pay attention anyway
}


