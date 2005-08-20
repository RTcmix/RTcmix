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
#include <RTcmix.h>
#include <prototypes.h>
#include <sndlibsupport.h>
#include <byte_routines.h>
#include <Instrument.h>
#include <BusSlot.h>
#include <ugens.h>
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

static inline long lmin(long a, long b) { return a < b ? a : b; }


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
   int   fdindex;
   off_t offset;

   fdindex = inst->_input.fdIndex;

   if (fdindex == NO_DEVICE_FDINDEX || RTcmix::isInputAudioDevice(fdindex)) {
      fprintf(stdin,
            "rtinrepos: request to reposition input, but input is not a file.");
	  return -1;
   }

   offset = RTcmix::seekInputFile(fdindex, frames, inst->_input.inputchans, whence);

   switch (whence) {
      case SEEK_SET:
         assert(offset >= 0);
         inst->_input.fileOffset = offset;
         break;
      case SEEK_CUR:
         inst->_input.fileOffset += offset;
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

off_t 
RTcmix::seekInputFile(int fdIndex, int frames, int chans, int whence)
{
   const int format = inputFileTable[fdIndex].data_format;
   const int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(format);

   off_t bytes = frames * chans * bytes_per_samp;

   switch (whence) {
      case SEEK_SET:
         assert(bytes >= 0);
         bytes += inputFileTable[fdIndex].data_location;
         break;
      case SEEK_CUR:
         break;
	  default:
         break;
   }
   return bytes;
}


/* -------------------- RTcmix::readFromAuxBus -------- [was get_aux_in] --- */
void
RTcmix::readFromAuxBus(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of auxin-bus chan nums from inst */
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

      ::copy_one_buf_to_interleaved_buf(dest, src, dest_chans, n, dest_frames);
   }

//   return 0;
}


/* ------------------------RTcmix::readFromAudioDevice [was get_audio_in] --- */
void
RTcmix::readFromAudioDevice(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
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

//   return 0;
}


/* ----------------------------------------------------- read_float_samps --- */
static int
read_float_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      long        cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                                    /* (or NULL to fill all chans) */
      short       src_chans         /* number of in-bus chans to copy */
      )
{
   static float *fbuf = NULL;
   
   const int bytes_per_samp = sizeof(float);

   if (fbuf == NULL) {     /* 1st time, so allocate interleaved float buffer */
      fbuf = (float *) malloc((size_t) RTcmix::bufsamps() * MAXCHANS * bytes_per_samp);
      if (fbuf == NULL) {
         perror("read_float_samps (malloc)");
         exit(1);
      }
   }
   char *bufp = (char *) fbuf;

   const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
   const long bytes_remaining = endbyte - cur_offset;
   const int extra_bytes = (bytes_requested > bytes_remaining)
                           ? bytes_requested - bytes_remaining : 0;
   ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);

   while (bytes_to_read > 0) {
      ssize_t bytes_read = read(fd, bufp, bytes_to_read);
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
   bytes_to_read += extra_bytes;
   while (bytes_to_read > 0) {
      (* (float *) bufp) = 0.0;
      bufp += bytes_per_samp;
      bytes_to_read -= bytes_per_samp;
   }

   /* Copy interleaved file buffer to dest buffer, with bus mapping. */

   const int src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      const int chan = n;
#else
      const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
      int j = n;
      for (int i = chan; i < src_samps; i += file_chans, j += dest_chans)
         dest[j] = (BUFTYPE) fbuf[i];
   }

#if MUS_LITTLE_ENDIAN
   const bool swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   const bool swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
   if (swap) {
      for (int i = 0; i < src_samps; i++)
         byte_reverse4(&dest[i]);
   }

   return 0;
}


/* ----------------------------------------------------- read_24bit_samps --- */
static int
read_24bit_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      long        cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                                    /* (or NULL to fill all chans) */
      short       src_chans         /* number of in-bus chans to copy */
      )
{
   static unsigned char *cbuf = NULL;
   
   const int bytes_per_samp = 3;         /* 24-bit int */

   if (cbuf == NULL) {     /* 1st time, so allocate interleaved char buffer */
      size_t nbytes = RTcmix::bufsamps() * MAXCHANS * bytes_per_samp;
      cbuf = (unsigned char *) malloc(nbytes);
      if (cbuf == NULL) {
         perror("read_24bit_samps (malloc)");
         exit(1);
      }
   }
   unsigned char *bufp = cbuf;

   const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
   const long bytes_remaining = endbyte - cur_offset;
   const int extra_bytes = (bytes_requested > bytes_remaining)
                           ? bytes_requested - bytes_remaining : 0;
   ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);

   while (bytes_to_read > 0) {
      ssize_t bytes_read = read(fd, bufp, bytes_to_read);
      if (bytes_read == -1) {
         perror("read_24bit_samps (read)");
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
   bytes_to_read += extra_bytes;
   while (bytes_to_read > 0) {
      *bufp++ = 0;
      bytes_to_read--;
   }

   /* Copy interleaved file buffer to dest buffer, with bus mapping. */

   const int src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      const int chan = n;
#else
      const int chan = src_chan_list ? src_chan_list[n] : n;
#endif
      int i = chan * bytes_per_samp;
      const int incr = file_chans * bytes_per_samp;
      const int src_bytes = src_samps * bytes_per_samp;
      if (data_format == MUS_L24INT) {
         for (int j = n; i < src_bytes; i += incr, j += dest_chans) {
            int samp = (int) (((cbuf[i + 2] << 24)
                             + (cbuf[i + 1] << 16)
                             + (cbuf[i] << 8)) >> 8);
            dest[j] = (BUFTYPE) samp / (BUFTYPE) (1 << 8);
         }
      }
      else {   /* data_format == MUS_B24INT */
         for (int j = n; i < src_bytes; i += incr, j += dest_chans) {
            int samp = (int) (((cbuf[i] << 24)
                             + (cbuf[i + 1] << 16)
                             + (cbuf[i + 2] << 8)) >> 8);
            dest[j] = (BUFTYPE) samp / (BUFTYPE) (1 << 8);
         }
      }
   }

   return 0;
}


/* ----------------------------------------------------- read_short_samps --- */
static int
read_short_samps(
      int         fd,               /* file descriptor for open input file */
      int         data_format,      /* sndlib data format of input file */
      int         file_chans,       /* total chans in input file */
      long        cur_offset,       /* current file position before read */
      long        endbyte,          /* first byte following last file sample */
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
                                    /* (or NULL to fill all chans) */
      short       src_chans         /* number of in-bus chans to copy */
      )
{
   static short *sbuf = NULL;
   
   const int bytes_per_samp = 2;         /* short int */

   if (sbuf == NULL) {     /* 1st time, so allocate interleaved short buffer */
      sbuf = (short *) malloc((size_t) RTcmix::bufsamps() * MAXCHANS * bytes_per_samp);
      if (sbuf == NULL) {
         perror("read_short_samps (malloc)");
         exit(1);
      }
   }
   char *bufp = (char *) sbuf;

   const int bytes_requested = dest_frames * file_chans * bytes_per_samp;
   const long bytes_remaining = endbyte - cur_offset;
   const int extra_bytes = (bytes_requested > bytes_remaining)
                           ? bytes_requested - bytes_remaining : 0;
   ssize_t bytes_to_read = lmin(bytes_remaining, bytes_requested);

   while (bytes_to_read > 0) {
      ssize_t bytes_read = read(fd, bufp, bytes_to_read);
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
   bytes_to_read += extra_bytes;
   while (bytes_to_read > 0) {
      (* (short *) bufp) = 0;
      bufp += bytes_per_samp;
      bytes_to_read -= bytes_per_samp;
   }

   /* Copy interleaved file buffer to dest buffer, with bus mapping. */

#if MUS_LITTLE_ENDIAN
   const bool swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   const bool swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif

   const int src_samps = dest_frames * file_chans;

   for (int n = 0; n < dest_chans; n++) {
#ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT
      const int chan = n;
#else
      const int chan = src_chan_list ? src_chan_list[n] : n;
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
/* Formerly used only by objlib/SoundIn.C. */
#ifdef NOMORE
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
   else if (IS_24BIT_FORMAT(data_format)) {
      status = read_24bit_samps(fd, data_format, file_chans,
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
#endif


/* ---------------------------RTcmix::readFromInputFile [was get_file_in ]--- */
void
RTcmix::readFromInputFile(
      BufPtr      dest,             /* interleaved buffer from inst */
      int         dest_chans,       /* number of chans interleaved */
      int         dest_frames,      /* frames in interleaved buffer */
      const short src_chan_list[],  /* list of in-bus chan numbers from inst */
      short       src_chans,        /* number of in-bus chans to copy */
      int		  fdIndex,			/* index into input file desc. array */
	  off_t		  *pFileOffset)		/* ptr to inst's file offset (updated) */
{
   int   status;

#ifndef IGNORE_BUS_COUNT_FOR_FILE_INPUT
   assert(dest_chans >= src_chans);
#endif

   /* File opened by earlier call to rtinput. */
   int fd = inputFileTable[fdIndex].fd;
   const int file_chans = inputFileTable[fdIndex].chans;
   const int data_format = inputFileTable[fdIndex].data_format;
   const int bytes_per_samp = ::mus_data_format_to_bytes_per_sample(data_format);
   const long endbyte = inputFileTable[fdIndex].endbyte;

   assert(file_chans <= MAXCHANS);
   assert(file_chans >= dest_chans);

   if (lseek(fd, *pFileOffset, SEEK_SET) == -1) {
      perror("RTcmix::readFromInputFile (lseek)");
      exit(1);
   }

   if (inputFileTable[fdIndex].is_float_format) {
      status = ::read_float_samps(fd, data_format, file_chans,
                                *pFileOffset, endbyte,
                                dest, dest_chans, dest_frames,
                                src_chan_list, src_chans);
   }
   else if (IS_24BIT_FORMAT(data_format)) {
      status = ::read_24bit_samps(fd, data_format, file_chans,
                                *pFileOffset, endbyte,
                                dest, dest_chans, dest_frames,
                                src_chan_list, src_chans);
   }
   else {
      status = ::read_short_samps(fd, data_format, file_chans,
                                *pFileOffset, endbyte,
                                dest, dest_chans, dest_frames,
                                src_chan_list, src_chans);
   }

   /* Advance saved offset by the number of samples (not frames) read.
      Note that this includes samples in channels that were read but
      not copied into the dest buffer!
   */
   *pFileOffset += dest_frames * file_chans * bytes_per_samp;

//   return status;
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
Instrument::rtgetin(float		*inarr,  /* interleaved array of <inputchans> */
        			Instrument	*inst,
        			int			nsamps)         /* samps, not frames */
{
   const int inchans = inst->inputChannels();    /* total in chans inst expects */
   const BusSlot *bus_config = inst->getBusSlot();
   const short in_count = bus_config->in_count;
   const int fdindex = inst->_input.fdIndex;
   const int frames = nsamps / inchans;

   assert(inarr != NULL);

   if (frames > RTcmix::bufsamps()) {
      die(inst->name(), "Internal Error: rtgetin: nsamps out of range!");
	  return -1;
   }

   if (fdindex == NO_DEVICE_FDINDEX) {               /* input from aux buses */
      const short *auxin = bus_config->auxin;        /* auxin channel list */
      const short auxin_count = bus_config->auxin_count;

      assert(auxin_count > 0);

      RTcmix::readFromAuxBus(inarr, inchans, frames, auxin, auxin_count, inst->output_offset);
   }
   else if (RTcmix::isInputAudioDevice(fdindex)) {  /* input from mic/line */
      const short *in = bus_config->in;              /* in channel list */

      assert(in_count > 0);

      RTcmix::readFromAudioDevice(inarr, inchans, frames, in, in_count, inst->output_offset);
   }
   else {                                            /* input from file */
      const short *in = bus_config->in;              /* in channel list */

      assert(in_count > 0);

      RTcmix::readFromInputFile(inarr, inchans, frames, in, in_count,
	  				            fdindex, &inst->_input.fileOffset);
   }

   return nsamps;   // this seems pointless, but no insts pay attention anyway
}


