#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <byte_routines.h>
#include <sndlibsupport.h>
#include <audio_port.h>

/* revision of Dave Topper's play program, by John Gibson, 6/99.
   rev'd again, for v2.3  -JGG, 2/26/00.
*/

#define PROGNAME  "cmixplay"

/* Tradeoff: larger BUF_FRAMES and NUM_FRAGMENTS make playback more robust
   on a loaded machine or over a network, at the cost of elapsed seconds
   display running ahead of playback.
   If user runs with robust flag (-r), code below increases buffer params
   by ROBUST_FACTOR.
*/

#define BUF_FRAMES          1024 * 2
#define ROBUST_FACTOR       4

#define NUM_ZERO_BUFS       8

#define ALL_CHANS           -1

/* #define DEBUG */
#define VERBOSE             1       /* if true, print buffer size */

#define UNSUPPORTED_DATA_FORMAT_MSG "\
%s: samples in an unsupported data format. \n\
(%s will play 16-bit linear or 32-bit floating point samples, \n\
in either byte order.) \n"

#define NEED_FACTOR_MSG "\
This floating-point file doesn't have up-to-date peak stats in its header. \n\
Either run sndpeak on the file or supply a rescale factor when you play.   \n\
(And watch out for your ears!)\n"

#define FORCE_FACTOR_MSG "\
Since this file header has no up-to-date peak stats, there's no way to  \n\
guarantee that your rescale factor won't cause painfully loud playback. \n\
If you really want to play the file with this factor anyway, use the    \n\
\"--force\" flag on the command line. But be careful with your ears!\n"



/* ---------------------------------------------------------------- usage --- */

#define USAGE_MSG "\
Usage: %s [options] filename  \n\
       options:  -s NUM    start time                      \n\
                 -e NUM    end time                        \n\
                 -d NUM    duration                        \n\
                 -f NUM    rescale factor for float files  \n\
                 -c NUM    channel (starting from 0)       \n\
                 -r        robust - larger audio buffers   \n\
                 -q        quiet - don't print anything    \n\
                 --force   use rescale factor even if peak \n\
                              amp of float file unknown    \n\
          Note:  -d ignored if you also give -e            \n\
"

static void
usage()
{
   fprintf(stderr, USAGE_MSG, PROGNAME);
   exit(1);
}


/* --------------------------------------------------------- write_buffer --- */
static int
write_buffer(int ports[], char *buf, int datum_size, int nframes, int nchans)
{
   int      i, j, n, buf_bytes;
   ssize_t  bytes_written;

#ifdef MONO_DEVICES
   static short *tmpbuf = NULL;

   buf_bytes = nframes * datum_size;

   if (tmpbuf == NULL) {                   /* first time, so allocate */
      tmpbuf = malloc((size_t)buf_bytes);
      assert(tmpbuf != NULL);
   }
   for (n = 0; n < nchans; n++) {
      for (i = 0, j = n; i < nframes; i++, j += nchans)
         tmpbuf[i] = ((short *)buf)[j];
      bytes_written = write(ports[n], tmpbuf, buf_bytes);
      if (bytes_written == -1) {
         perror("audio write");
         return -1;
      }
   }

#else /* !MONO_DEVICES */

 #ifdef USE_CARD_CHANS

   static int   card_chans = -1;
   static short *tmpbuf = NULL;

   if (tmpbuf == NULL) {                      /* first time, so check */
      card_chans = get_card_max_chans(ports[0]);
      assert(card_chans >= nchans);
   }

   /* If file has fewer chans than card, copy file chans into a temp
      interleaved array of card_chans, then zero-pad the rest of the
      chans in the larger array.
   */
   if (card_chans > nchans) {
      if (tmpbuf == NULL) {                   /* first time, so allocate */
         tmpbuf = calloc(nframes * card_chans, sizeof(short));   /* zeros mem */
         assert(tmpbuf != NULL);
      }
      for (n = 0; n < nchans; n++) {
         int k;
         j = k = n;
         for (i = 0; i < nframes; i++) {
            tmpbuf[k] = ((short *)buf)[j];
            j += nchans;
            k += card_chans;
         }
      }
      buf_bytes = nframes * card_chans * datum_size;
      bytes_written = write(ports[0], tmpbuf, buf_bytes);
   }
   else {
      buf_bytes = nframes * nchans * datum_size;
      bytes_written = write(ports[0], buf, buf_bytes);
   }

 #else /* !USE_CARD_CHANS */

   buf_bytes = nframes * nchans * datum_size;
   bytes_written = write(ports[0], buf, buf_bytes);

 #endif /* !USE_CARD_CHANS */

   if (bytes_written == -1) {
      perror("audio write");
      return -1;
   }
#endif /* !MONO_DEVICES */

   return 0;
}


/* ---------------------------------------------------------- close_ports --- */
static void
close_ports(int ports[], int nchans)
{
#ifdef MONO_DEVICES
   int   n;

   for (n = 0; n < nchans; n++)
      close(ports[n]);
#else /* !MONO_DEVICES */
   close(ports[0]);
#endif /* !MONO_DEVICES */
}


/* ----------------------------------------------------------------- main --- */
int main(int argc, char *argv[])
{
   int         i, n, fd, datum_size, second, status, fragments;
   int         quiet, robust, force, is_float, swap, play_chan;
   int         header_type, data_format, data_location, srate, in_chans, nsamps;
   int         buf_bytes, buf_samps, buf_frames, buf_start_frame;
   int         start_frame, end_frame, nframes;
   int         afd[MAXCHANS];
   long        skip_bytes;
   float       start_time, end_time, request_dur, buf_start_time, factor, dur;
   char        *sfname, *bufp;
   short       *sbuf;
   float       *fbuf;
   struct stat statbuf;
   SFComment   sfc;

   if (argc < 2)
      usage();

   sfname = NULL;
   quiet = robust = force = 0;
   start_time = end_time = request_dur = factor = 0.0;
   play_chan = ALL_CHANS;

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 's':
               if (++i >= argc)
                  usage();
               start_time = atof(argv[i]);
               break;
            case 'e':
               if (++i >= argc)
                  usage();
               end_time = atof(argv[i]);
               break;
            case 'd':
               if (++i >= argc)
                  usage();
               request_dur = atof(argv[i]);
               break;
            case 'f':
               if (++i >= argc)
                  usage();
               factor = atof(argv[i]);
               break;
            case 'c':
               if (++i >= argc)
                  usage();
               play_chan = atoi(argv[i]);
               break;
            case 'r':
               robust = 1;
               break;
            case 'q':
               quiet = 1;
               break;
            case '-':
               if (strcmp(arg, "--force") == 0)
                  force = 1;
               else
                  usage();
               break;
            default:  
               usage();
         }
      }
      else
         sfname = arg;
   }
   if (sfname == NULL) {
      fprintf(stderr, "You didn't give a valid filename.\n");
      exit(1);
   }

   /* input validation */

   if (start_time < 0.0) {
      fprintf(stderr, "Start time must be positive.\n");
      exit(1);
   }
   if (start_time > 0.0 && end_time > 0.0 && start_time >= end_time) {
      fprintf(stderr, "Start time must be less than end time.\n");
      exit(1);
   }
   if (end_time == 0.0 && request_dur > 0.0)
      end_time = start_time + request_dur;
   if (factor < 0.0) {
      fprintf(stderr, "Rescale factor must be greater than 0.\n");
      exit(1);
   }

   /* see if file exists and we can read it */

   fd = open(sfname, O_RDONLY);
   if (fd == -1) {
      fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
      exit(1);
   }

   /* make sure it's a regular file or symbolic link */

   if (fstat(fd, &statbuf) == -1) {
      fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
      exit(1);
   }
   if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
      fprintf(stderr, "\"%s\" is not a regular file or a link.\n", sfname);
      exit(1);
   }

   /* read header and gather info */

   if (sndlib_read_header(fd) == -1) {
      fprintf(stderr, "Can't read \"%s\"!\n", sfname);
      exit(1);
   }
   header_type = c_snd_header_type();
   if (NOT_A_SOUND_FILE(header_type)) {
      fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
      exit(1);
   }
   data_format = c_snd_header_format();
   if (!SUPPORTED_DATA_FORMAT(data_format)) {
      fprintf(stderr, UNSUPPORTED_DATA_FORMAT_MSG, sfname, PROGNAME);
      exit(1);
   }
   is_float = IS_FLOAT_FORMAT(data_format);
#ifdef SNDLIB_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
   data_location = c_snd_header_data_location();
   srate = c_snd_header_srate();
   in_chans = c_snd_header_chans();
   datum_size = c_snd_header_datum_size();           /* bytes per sample */
   nsamps = c_snd_header_data_size();                /* samples, not frames */
   dur = (float)(nsamps / in_chans) / (float)srate;

   /* more input validation */

   if (start_time >= dur) {
      fprintf(stderr, "Start time must be less than duration of file.\n");
      exit(1);
   }
   if (end_time > 0.0 && end_time > dur) {
      if (!quiet)
         printf("Note: Your end time was later than the end of file.\n");
      /* but continue anyway */
      end_time = 0.0;
   }
   if (in_chans > MAXCHANS) {
      fprintf(stderr,
              "Not configured to play files with more than %d channels.\n",
              MAXCHANS);
      exit(1);
   }
   if (play_chan >= in_chans) {
      fprintf(stderr, "You asked to play channel %d of a %d-channel file.\n",
              play_chan, in_chans);
      exit(1);
   }

   /* prepare for playing a float file */

   if (is_float) {
      int stats_valid;

      if (sndlib_get_current_header_comment(fd, &sfc) == -1) {
         fprintf(stderr, "Can't read header comment!\n");
         exit(1);
      }
      stats_valid = (SFCOMMENT_PEAKSTATS_VALID(&sfc)
                     && statbuf.st_mtime <= sfc.timetag + 2);

      if (stats_valid) {
         float peak = 0.0;
         for (n = 0; n < in_chans; n++)
            if (sfc.peak[n] > peak)
               peak = sfc.peak[n];
         if (peak > 0.0) {
            float tmp_factor = 32767.0 / peak;
            if (factor) {
               if (factor > tmp_factor) {
                  fprintf(stderr,
                          "Your rescale factor (%g) would cause clipping.\n",
                                                                     factor);
                  exit(1);
               }
            }
            else
               factor = tmp_factor > 1.0 ? 1.0 : tmp_factor;
         }
         else
            stats_valid = 0;              /* better not to believe this peak */
      }
      if (!stats_valid) {      /* Note: stats_valid can change in prev block */
         if (factor == 0.0) {
            fprintf(stderr, NEED_FACTOR_MSG);
            exit(1);
         }
         else if (!force) {
            fprintf(stderr, FORCE_FACTOR_MSG);
            exit(1);
         }
      }
   }

   /* open the audio output port */

   buf_frames = BUF_FRAMES;
   if (robust)
      buf_frames *= ROBUST_FACTOR;

   fragments = NUM_FRAGMENTS;
   if (robust)
      fragments *= ROBUST_FACTOR;

   /* Note: This will set number of chans depending on USE_CARD_CHANS define. */
   status = open_ports(0, 0, NULL, in_chans, MAXCHANS, afd, VERBOSE,
                                      (float) srate, fragments, &buf_frames);
   if (status == -1)
      exit(1);

   /* allocate buffer(s) */

   buf_samps = buf_frames * in_chans;
// FIXME: 24-bit audio...
   buf_bytes = buf_samps * sizeof(short);

// FIXME: 24-bit audio...
   sbuf = (short *)malloc((size_t)buf_bytes);
   if (sbuf == NULL) {
      perror("short buffer malloc");
      exit(1);
   }
   if (is_float) {
      fbuf = (float *)malloc((size_t)(buf_samps * sizeof(float)));
      if (fbuf == NULL) {
         perror("float buffer malloc");
         exit(1);
      }
      bufp = (char *)fbuf;
   }
   else
      bufp = (char *)sbuf;

   /* print out stuff, if not running in quiet mode */

   if (!quiet) {
      printf("File: %s\n", sfname);
      printf("rate: %d  chans: %d\n", srate, in_chans);
      if (start_time > 0.0)
         printf("Skipping %g seconds.\n", start_time);
      if (end_time > 0.0)
         printf("Ending at %g seconds.\n", end_time);
      if (is_float)
         printf("Rescale factor: %g\n", factor);
      if (robust)
         printf("Warning: \"robust\" mode causes second count to run ahead.\n");
      printf("Time: ");
      fflush(stdout);
   }

   /* init variables used in playback loop */

   if (start_time > 0.0) {
      start_frame = (int)(start_time * srate);
      skip_bytes = (long)(start_frame * in_chans * datum_size);
   }
   else
      start_frame = skip_bytes = 0;

   if (end_time > 0.0)
      end_frame = (int)(end_time * srate);
   else
      end_frame = nsamps / in_chans;

   buf_frames = buf_samps / in_chans;

   if (!quiet) {
      buf_start_time = start_time;
      second = (int)buf_start_time;
   }

   /* seek to start_time on input file */

   if (lseek(fd, data_location + skip_bytes, SEEK_SET) == -1) {
      perror("lseek");
      exit(1);
   }

   /* read input samples until end_frame, and copy to audio port,
      byte-swapping and converting from floats as necessary.
      If not running in quiet mode, print seconds as they elapse.
   */
   buf_start_frame = start_frame;
   for (  ; buf_start_frame < end_frame; buf_start_frame += nframes) {
      int  samps_read;
      long bytes_read;

      if (buf_start_frame + buf_frames > end_frame) {      /* last buffer */
         int samps = (end_frame - buf_start_frame) * in_chans;
         bytes_read = read(fd, bufp, samps * datum_size);
      }
      else
         bytes_read = read(fd, bufp, buf_samps * datum_size);
      if (bytes_read == -1) {
         perror("read");
         close_ports(afd, in_chans);
         exit(1);
      }
      if (bytes_read == 0)          /* EOF, somehow */
         break;

      samps_read = bytes_read / datum_size;
      nframes = samps_read / in_chans;

      if (is_float) {
         if (swap) {
            for (i = 0; i < samps_read; i++) {
               byte_reverse4(&fbuf[i]);
               fbuf[i] *= factor;
               sbuf[i] = (short)fbuf[i];
            }
         }
         else {
            for (i = 0; i < samps_read; i++) {
               fbuf[i] *= factor;
               sbuf[i] = (short)fbuf[i];
            }
         }
      }
      else if (swap) {
         for (i = 0; i < samps_read; i++)
            sbuf[i] = reverse_int2(&sbuf[i]);
      }

      for (i = samps_read; i < buf_samps; i++)
         sbuf[i] = 0;

      /* Not efficient, but easy way to play just 1 of several chans. */
      if (play_chan > ALL_CHANS) {
         for (i = 0; i < samps_read; i += in_chans) {
            short samp = sbuf[i + play_chan];
            for (n = 0; n < in_chans; n++)
               sbuf[i + n] = samp;
         }
      }

      status = write_buffer(afd, (char *)sbuf, AUDIO_DATUM_SIZE, nframes,
                                                                  in_chans);
      if (status == -1)
         exit(1);

      if (!quiet) {
         buf_start_time += (float)nframes / (float)srate;
         if (buf_start_time > second) {
            printf("%d ", second);
            fflush(stdout);
            second++;
         }
      }
   }

   /* write buffers of zeros to prevent clicks */
   for (i = 0; i < buf_samps; i++)
      sbuf[i] = 0;
   for (i = 0; i < NUM_ZERO_BUFS; i++) {
      status = write_buffer(afd, (char *)sbuf, AUDIO_DATUM_SIZE, nframes,
                                                                  in_chans);
      if (status == -1)
         exit(1);
   }

   close_ports(afd, in_chans);
   close(fd);

   if (!quiet)
      printf("\n");

   return 0;
}


#endif /* LINUX */

