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
#include <prototypes.h>
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
Instrument::rtinrepos(Instrument *inst, int frames, int whence)
{
   int   fdindex, fd, bytes_per_samp;
   off_t bytes;

   fdindex = inst->fdIndex;

   if (fdindex == NO_DEVICE_FDINDEX || inputFileTable[fdindex].is_audio_dev) {
      fprintf(stdin,
            "rtinrepos: request to reposition input, but input is not a file.");
   }

   fd = inputFileTable[fdindex].fd;

   if (inputFileTable[fdindex].is_float_format)
      bytes_per_samp = sizeof(float);
   else
      bytes_per_samp = 2;

   bytes = frames * inst->inputchans * bytes_per_samp;

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
      int  	      output_offset)
{
   assert(dest_chans >= src_chans);

   for (int n = 0; n < src_chans; n++) {
      int chan = src_chan_list[n];

      BufPtr src = aux_buffer[chan];
      assert(src != NULL);

      /* The inst might be playing only the last part of a buffer. If so,
         we want it to read the corresponding segment of the aux buffer.
      */
      src += output_offset;

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
      int  		  output_offset)
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
      src += output_offset;

      copy_one_buf_to_interleaved_buf(dest, src, dest_chans, n, dest_frames);
   }

   return 0;
}


/* ----------------------------------------------------- read_float_samps --- */
static int
read_float_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
                                    /* (or NULL to fill all chans) */
      short       src_chans         /* number of in-bus chans to copy */
      )
{
   int            seeked, src_samps, swap, bytes_per_samp;
   ssize_t        bytes_to_read, bytes_read;
   char           *bufp;
   static float   *fbuf = NULL;
   
   bytes_per_samp = sizeof(float);

   if (fbuf == NULL) {     /* 1st time, so allocate interleaved float buffer */
      fbuf = (float *) malloc((size_t) RTBUFSAMPS * MAXCHANS * bytes_per_samp);
      if (fbuf == NULL) {
         perror("read_float_samps (malloc)");
         exit(1);
      }
   }
   bufp = (char *) fbuf;

   bytes_to_read = dest_frames * file_chans * bytes_per_samp;

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
      bufp += bytes_per_samp;
      bytes_to_read -= bytes_per_samp;
   }

   /* Copy interleaved file buffer to dest buffer, with bus mapping. */

   src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      int chan = n;
#else
      int chan = src_chan_list? src_chan_list[n] : n;
#endif
      int j = n;
      for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
         dest[j] = (BUFTYPE) fbuf[i];
   }

#if MUS_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
   if (swap) {
      for (int i = 0; i < src_samps; i++)
         byte_reverse4(&dest[i]);
   }

   return 0;
}


/* ----------------------------------------------------- read_short_samps --- */
static int
read_short_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
                                    /* (or NULL to fill all chans) */
      short       src_chans         /* number of in-bus chans to copy */
      )
{
   int            seeked, src_samps, swap, bytes_per_samp;
   ssize_t        bytes_to_read, bytes_read;
   char           *bufp;
   static short   *sbuf = NULL;
   
   bytes_per_samp = 2;         /* short int */

   if (sbuf == NULL) {     /* 1st time, so allocate interleaved short buffer */
      sbuf = (short *) malloc((size_t) RTBUFSAMPS * MAXCHANS * bytes_per_samp);
      if (sbuf == NULL) {
         perror("read_short_samps (malloc)");
         exit(1);
      }
   }
   bufp = (char *) sbuf;

   bytes_to_read = dest_frames * file_chans * bytes_per_samp;

   while (bytes_to_read > 0) {
      bytes_read = read(fd, bufp, bytes_to_read);
      if (bytes_read == -1) {
         perror("read_short_samps (read)");
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
      (* (short *) bufp) = 0;
      bufp += bytes_per_samp;
      bytes_to_read -= bytes_per_samp;
   }

   /* Copy interleaved file buffer to dest buffer, with bus mapping. */

#if MUS_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif

   src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      int chan = n;
#else
      int chan = src_chan_list? src_chan_list[n] : n;
#endif
      if (swap) {
         int j = n;
         for (int i = chan; i < src_samps; i += file_chans, j += dest_chans) {
            sbuf[i] = reverse_int2(&sbuf[i]);
            dest[j] = (BUFTYPE) sbuf[i];
         }
      }
      else {
         int j = n;
         for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
            dest[j] = (BUFTYPE) sbuf[i];
      }
   }

   return 0;
}


/* ----------------------------------------------------------- read_samps --- */
/* Currently used only by objlib/SoundIn.C. */
int
read_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames       /* frames in interleaved buffer */
      )
{
   int   status;

   assert(file_chans <= MAXCHANS);
   assert(file_chans >= dest_chans);

   if (IS_FLOAT_FORMAT(data_format)) {
      status = read_float_samps(fd, data_format, file_chans,
                                dest, dest_chans, dest_frames,
                                NULL, 0);
   }
   else {
      status = read_short_samps(fd, data_format, file_chans,
                                dest, dest_chans, dest_frames,
                                NULL, 0);
   }

   return status;
}


/* ---------------------------------------------------------- get_file_in --- */
static int
get_file_in(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      short       src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      int		  fdIndex,			/* index into input file desc. array */
	  off_t		  *pFileOffset)		/* ptr to inst's file offset (updated) */
{
   int   status, fd, data_format, bytes_per_samp, file_chans;

#ifndef IGNORE_BUS_COUNT_FOR_FILE_INPUT
   assert(dest_chans >= src_chans);
#endif

   /* File opened by earlier call to rtinput. */
   fd = inputFileTable[fdIndex].fd;
   file_chans = inputFileTable[fdIndex].chans;
   assert(file_chans <= MAXCHANS);
   assert(file_chans >= dest_chans);
   data_format = inputFileTable[fdIndex].data_format;
   bytes_per_samp = mus_data_format_to_bytes_per_sample(data_format);

   if (lseek(fd, *pFileOffset, SEEK_SET) == -1) {
      perror("get_file_in (lseek)");
      exit(1);
   }

   if (inputFileTable[fdIndex].is_float_format) {
      status = read_float_samps(fd, data_format, file_chans,
                                dest, dest_chans, dest_frames,
                                src_chan_list, src_chans);
   }
   else {
      status = read_short_samps(fd, data_format, file_chans,
                                dest, dest_chans, dest_frames,
                                src_chan_list, src_chans);
   }

   /* Advance saved offset by the number of samples (not frames) read.
      Note that this includes samples in channels that were read but
      not copied into the dest buffer!
   */
   *pFileOffset += dest_frames * file_chans * bytes_per_samp;

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
Instrument::rtgetin(float      *inarr,         /* interleaved array of <inputchans> */
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

      status = get_aux_in(inarr, inchans, frames, auxin, auxin_count, inst->output_offset);
   }
   else if (inputFileTable[fdindex].is_audio_dev) {  /* input from mic/line */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_audio_in(inarr, inchans, frames, in, in_count, inst->output_offset);
   }
   else {                                            /* input from file */
      short *in = inst->bus_config->in;              /* in channel list */

      assert(in_count > 0 && auxin_count == 0);

      status = get_file_in(inarr, inchans, frames, in, in_count,
	  					   inst->fdIndex, &inst->fileOffset);
   }

   return nsamps;   // this seems pointless, but no insts pay attention anyway
}


