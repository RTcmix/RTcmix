/* RTcmix  - Copyright (C) 2001  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* revision of Dave Topper's play program, by John Gibson, 6/99.
   rev'd again, for v2.3  -JGG, 2/26/00.
   rev'd again, for OSX support  -JGG, 12/01.
*/
#if defined(LINUX) || defined(MACOSX)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <byte_routines.h>
#include <sndlibsupport.h>
#include <audio_port.h>

#define PROGNAME  "cmixplay"

/* Tradeoff: larger BUF_FRAMES and NUM_FRAGMENTS make playback more robust
   on a loaded machine or over a network, at the cost of elapsed seconds
   display running ahead of playback.
   If user runs with robust flag (-r), code below increases buffer params
   by ROBUST_FACTOR.
*/

#define BUF_FRAMES          1024 * 2
#ifdef LINUX
#define ROBUST_FACTOR       4
#endif
#ifdef MACOSX
// 4 glitches ... why?
#define ROBUST_FACTOR       2
#endif

#define SKIP_SECONDS        4.0     /* for fast-forward and rewind */
#define MARK_PRECISION      2       /* digits after decimal point to print
                                       for mark */
#define MARK_DELAY          0.1     /* report mark time earlier by this much,
                                       to make up for delay when typing key */

#define ALL_CHANS           -1

/* #define DEBUG */
#define VERBOSE             0       /* if true, print buffer size */

#define UNSUPPORTED_DATA_FORMAT_MSG "\
%s: samples in an unsupported data format. \n\
(%s will play 16-bit linear, 24-bit linear or 32-bit floating point samples, \n\
in either byte order.) \n"

#define NEED_FACTOR_MSG "\
This floating-point file doesn't have up-to-date peak stats in its header. \n\
Either run sndpeak on the file or supply a rescale factor when you play.   \n\
(And watch out for your ears!)\n"

#define CLIPPING_FORCE_FACTOR_MSG "\
Your rescale factor (%g) would cause clipping.  If you really want to  \n\
play the file with this factor anyway, use the \"--force\" flag on the \n\
command line.  But be careful with your ears!\n"

#define NO_STATS_FORCE_FACTOR_MSG "\
Since this file header has no up-to-date peak stats, there's no way to  \n\
guarantee that your rescale factor won't cause painfully loud playback. \n\
If you really want to play the file with this factor anyway, use the    \n\
\"--force\" flag on the command line.  But be careful with your ears!\n"

#ifdef MACOSX
/* We ignore these, but need them to link with audio_port.o */
int play_audio = 1;
typedef float *BufPtr;
BufPtr audioin_buffer[1];
BufPtr out_buffer[1];
#endif

static int print_minutes_seconds = 0;  /* print time as 1:20 instead of 80 */

struct termios saved_term_attributes;

void reset_input_mode(void)
{
   tcsetattr(STDIN_FILENO, TCSANOW, &saved_term_attributes);
}


/* ---------------------------------------------------------------- usage --- */

#define USAGE_MSG "\
Usage: %s [options] filename  \n\
       options:  -s NUM    start time                           \n\
                 -e NUM    end time                             \n\
                 -d NUM    duration                             \n\
                 -f NUM    rescale factor                       \n\
                 -c NUM    channel (starting from 0)            \n\
                 -h        view this help screen                \n\
                 -k        disable hotkeys (see below)          \n\
                 -t NUM    time to skip for rewind, fast-forward\n\
                              (default is 4 seconds)            \n\
                 -r        robust - larger audio buffers        \n\
                 -q        quiet - don't print anything         \n\
                 --force   use rescale factor even if peak      \n\
                              amp of float file unknown         \n\
          Notes: -d ignored if you also give -e                 \n\
                 Times can be given as seconds or 0:00.0        \n\
                    If the latter, prints time in same way.     \n\
                 Hotkeys: 'f': fast-forward, 'r': rewind,       \n\
                          'm': mark (print current buffer start time)\n\
                          't': toggle time display              \n\
                 To stop playing: cntl-D or cntl-C              \n\
"

static void
usage()
{
   fprintf(stderr, USAGE_MSG, PROGNAME);
   exit(EXIT_FAILURE);
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

   if (card_chans == -1) {                      /* first time, so check */
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
      /* If input is mono, copy the signal into both channels of the first
         stereo pair.
      */
      if (nchans == 1) {
         for (i = j = 0; i < nframes; i++, j += card_chans) {
            tmpbuf[j] = ((short *)buf)[i];
            tmpbuf[j + 1] = tmpbuf[j];
         }
      }
      else {
         for (n = 0; n < nchans; n++) {
            int k;
            j = k = n;
            for (i = 0; i < nframes; i++) {
               tmpbuf[k] = ((short *)buf)[j];
               j += nchans;
               k += card_chans;
            }
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


/* ----------------------------------------------------- make_time_string --- */
static char *
make_time_string(float seconds, int precision)
{
   int         minutes, secs;
   static char buf[32];

   minutes = (int)seconds / 60;
   secs = (int)seconds % 60;
   if (precision > 0) {
      char  tmp[16], *p;
      float frac = seconds - (int)seconds;
      snprintf(tmp, 16, "%.*f", precision, frac);
      p = tmp + 1;      /* skip 0 before decimal point */
      snprintf(buf, 32, "%d:%02d%s", minutes, secs, p);
   }
   else
      snprintf(buf, 32, "%d:%02d", minutes, secs);
   return buf;
}


/* ---------------------------------------------------------- get_seconds --- */
static float
get_seconds(char timestr[])
{
   float seconds;
   char  *p, *str;

   str = strdup(timestr);
   if (str == NULL) {
      fprintf(stderr, "get_seconds: can't allocate string buffer.\n");
      exit(EXIT_FAILURE);
   }
   p = strchr(str, ':');
   if (p) {
      float minutes;
      *p = '\0';
      p++;        /* now str points to minutes str; p points to seconds */
      minutes = atof(str);
      seconds = atof(p);
      seconds += minutes * 60.0;
      print_minutes_seconds = 1;
   }
   else
      seconds = atof(str);
   free(str);

   return seconds;
}


/* ------------------------------------------------------- set_input_mode --- */
static void
set_input_mode(void)
{
   struct termios tattr;

   /* Make sure stdin is a terminal. */
   if (!isatty(STDIN_FILENO)) {
      fprintf(stderr, "Not a terminal.\n");
      exit(EXIT_FAILURE);
   }

   /* Save the terminal attributes so we can restore them at exit. */
   tcgetattr(STDIN_FILENO, &saved_term_attributes);
   atexit(reset_input_mode);

   /* Set terminal modes so fast-forward, rewind, etc. will work. */
   tcgetattr(STDIN_FILENO, &tattr);
   tattr.c_lflag &= ~(ICANON | ECHO);  /* Clear ICANON and ECHO. */
   tattr.c_cc[VMIN] = 0;               /* so read doesn't block; it */
   tattr.c_cc[VTIME] = 0;              /*    returns 0 if no input */
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}


/* ----------------------------------------------------------------- main --- */
int main(int argc, char *argv[])
{
   int         i, n, fd, datum_size, second, status, fragments, stats_valid;
   int         quiet, robust, force, is_float, is_24bit, swap, play_chan,
               hotkeys;
   int         header_type, data_format, data_location, srate, in_chans, nsamps;
   int         buf_bytes, buf_samps, buf_frames, buf_start_frame;
   int         start_frame, end_frame, nframes, skip_frames;
   long        skip_bytes;
   float       start_time, end_time, request_dur, buf_start_time, factor, dur;
   float       skip_time;
   char        *sfname, *bufp;
   unsigned char *cbuf;
   short       *sbuf;
   float       *fbuf;
   struct stat statbuf;
   SFComment   sfc;
#ifdef LINUX
   int         afd[MAXCHANS];
#endif
#ifdef MACOSX
   AudioDeviceID out_port;
#endif

   if (argc < 2)
      usage();

   sfname = NULL;
   fbuf = NULL;
   cbuf = NULL;
   quiet = robust = force = second = nframes = 0;
   start_time = end_time = buf_start_time = request_dur = factor = 0.0;
   play_chan = ALL_CHANS;
   hotkeys = 1;
   skip_time = SKIP_SECONDS;
   skip_frames = 0;     /* need srate to compute */

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 's':               /* start time */
               if (++i >= argc)
                  usage();
               start_time = get_seconds(argv[i]);
               break;
            case 'e':               /* end time */
               if (++i >= argc)
                  usage();
               end_time = get_seconds(argv[i]);
               break;
            case 'd':               /* duration */
               if (++i >= argc)
                  usage();
               request_dur = get_seconds(argv[i]);
               break;
            case 'f':               /* rescale factor (for float files) */
               if (++i >= argc)
                  usage();
               factor = atof(argv[i]);
               break;
            case 'c':               /* channel */
               if (++i >= argc)
                  usage();
               play_chan = atoi(argv[i]);
               break;
            case 'k':               /* disable hotkeys */
               hotkeys = 0;
               break;
            case 't':               /* disable hotkeys */
               if (++i >= argc)
                  usage();
               skip_time = get_seconds(argv[i]);
               break;
            case 'r':               /* robust buffer size */
               robust = 1;
               break;
            case 'q':               /* no printout */
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
      exit(EXIT_FAILURE);
   }

   /* input validation */

   if (start_time < 0.0) {
      fprintf(stderr, "Start time must be positive.\n");
      exit(EXIT_FAILURE);
   }
   if (start_time > 0.0 && end_time > 0.0 && start_time >= end_time) {
      fprintf(stderr, "Start time must be less than end time.\n");
      exit(EXIT_FAILURE);
   }
   if (end_time == 0.0 && request_dur > 0.0)
      end_time = start_time + request_dur;
   if (factor < 0.0) {
      fprintf(stderr, "Rescale factor must be greater than 0.\n");
      exit(EXIT_FAILURE);
   }
   if (hotkeys && skip_time <= 0.0) {
      fprintf(stderr, "Skip time must be greater than zero.\n");
      exit(EXIT_FAILURE);
   }

   /* see if file exists and we can read it */

   fd = open(sfname, O_RDONLY);
   if (fd == -1) {
      fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
      exit(EXIT_FAILURE);
   }

   /* make sure it's a regular file or symbolic link */

   if (fstat(fd, &statbuf) == -1) {
      fprintf(stderr, "%s: %s\n", sfname, strerror(errno));
      exit(EXIT_FAILURE);
   }
   if (!S_ISREG(statbuf.st_mode) && !S_ISLNK(statbuf.st_mode)) {
      fprintf(stderr, "\"%s\" is not a regular file or a link.\n", sfname);
      exit(EXIT_FAILURE);
   }

   /* read header and gather info */

   if (sndlib_read_header(fd) == -1) {
      fprintf(stderr, "Can't read \"%s\"!\n", sfname);
      exit(EXIT_FAILURE);
   }
   header_type = mus_header_type();
   if (NOT_A_SOUND_FILE(header_type)) {
      fprintf(stderr, "\"%s\" is probably not a sound file\n", sfname);
      exit(EXIT_FAILURE);
   }
   data_format = mus_header_format();
   if (!SUPPORTED_DATA_FORMAT(data_format)) {
      fprintf(stderr, UNSUPPORTED_DATA_FORMAT_MSG, sfname, PROGNAME);
      exit(EXIT_FAILURE);
   }
   is_float = IS_FLOAT_FORMAT(data_format);
   is_24bit = IS_24BIT_FORMAT(data_format);
#if MUS_LITTLE_ENDIAN
   swap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   swap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
   data_location = mus_header_data_location();
   srate = mus_header_srate();
   in_chans = mus_header_chans();
   datum_size = mus_header_data_format_to_bytes_per_sample();
   nsamps = mus_header_samples();                /* samples, not frames */
   dur = (float)(nsamps / in_chans) / (float)srate;

   /* more input validation */

   if (start_time >= dur) {
      fprintf(stderr, "Start time must be less than duration of file.\n");
      exit(EXIT_FAILURE);
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
      exit(EXIT_FAILURE);
   }
   if (play_chan >= in_chans) {
      fprintf(stderr, "You asked to play channel %d of a %d-channel file.\n",
              play_chan, in_chans);
      exit(EXIT_FAILURE);
   }

   /* prepare rescale factor */

   if (sndlib_get_current_header_comment(fd, &sfc) == -1) {
      fprintf(stderr, "Can't read header comment!\n");
      exit(EXIT_FAILURE);
   }
   stats_valid = (SFCOMMENT_PEAKSTATS_VALID(&sfc)
                  && sfcomment_peakstats_current(&sfc, fd));

   if (stats_valid) {
      float peak = 0.0;
      for (n = 0; n < in_chans; n++)
         if (sfc.peak[n] > peak)
            peak = sfc.peak[n];
      if (peak > 0.0) {
         float tmp_factor = 32767.0 / peak;
         if (factor) {
            if (factor > tmp_factor && !force) {
               fprintf(stderr, CLIPPING_FORCE_FACTOR_MSG, factor);
               exit(EXIT_FAILURE);
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
         if (is_float) {
            fprintf(stderr, NEED_FACTOR_MSG);
            exit(EXIT_FAILURE);
         }
         else
            factor = 1.0;
      }
      else if (!force) {
         if (is_float || factor > 1.0) {
            fprintf(stderr, NO_STATS_FORCE_FACTOR_MSG);
            exit(EXIT_FAILURE);
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

#ifdef LINUX
   /* Note: This will set number of chans depending on USE_CARD_CHANS define. */
   status = open_ports(0, 0, NULL, in_chans, MAXCHANS, afd, VERBOSE,
                                    (float) srate, fragments, &buf_frames);
#endif
#ifdef MACOSX
   status = open_macosx_ports(0, in_chans, NULL, &out_port, VERBOSE,
                                    (float) srate, fragments, &buf_frames);
#endif
   if (status == -1)
      exit(EXIT_FAILURE);

   /* allocate buffer(s) */

   buf_samps = buf_frames * in_chans;
// FIXME: 24-bit audio output...
   buf_bytes = buf_samps * sizeof(short);

   /* output buffer */
// FIXME: truncates 24-bit and float audio output; should feed OS X floats
   sbuf = (short *)malloc((size_t)buf_bytes);
   if (sbuf == NULL) {
      perror("short buffer malloc");
      exit(EXIT_FAILURE);
   }

   /* input buffer */
   if (is_float) {
      fbuf = (float *)malloc((size_t)(buf_samps * sizeof(float)));
      if (fbuf == NULL) {
         perror("float buffer malloc");
         exit(EXIT_FAILURE);
      }
      bufp = (char *)fbuf;
   }
   else if (is_24bit) {
      cbuf = (unsigned char *)malloc((size_t)buf_samps * datum_size);
      if (cbuf == NULL) {
         perror("24bit buffer malloc");
         exit(EXIT_FAILURE);
      }
      bufp = (char *)cbuf;
   }
   else
      bufp = (char *)sbuf;

   /* print out stuff, if not running in quiet mode */

   if (!quiet) {
      printf("File: %s\n", sfname);
      printf("Rate: %d Hz   Chans: %d\n", srate, in_chans);
      printf("Duration: %.*f seconds [%s]\n", MARK_PRECISION, dur,
                                       make_time_string(dur, MARK_PRECISION));
      if (start_time > 0.0)
         printf("Skipping %g seconds.\n", start_time);
      if (end_time > 0.0)
         printf("Ending at %g seconds.\n", end_time);
      printf("Rescale factor: %g\n", factor);
      if (robust)
         printf("Warning: \"robust\" mode causes second count to run ahead.\n");
      printf("Time: ");
      fflush(stdout);
   }

   /* set terminal up to handle hotkey input, etc. */
   if (hotkeys) {
      set_input_mode();
      skip_frames = (int)(skip_time * srate);
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

   buf_start_time = start_time;

   if (!quiet)
      second = (int)buf_start_time;

   /* seek to start_time on input file */

   if (lseek(fd, data_location + skip_bytes, SEEK_SET) == -1) {
      perror("lseek");
      exit(EXIT_FAILURE);
   }

   /* write buffers of zeros to audio output to prevent clicks */
   for (i = 0; i < buf_samps; i++)
      sbuf[i] = 0;
   for (i = 0; i < ZERO_FRAMES_BEFORE / buf_frames; i++) {
#ifdef LINUX
      status = write_buffer(afd, (char *)sbuf, AUDIO_DATUM_SIZE,
                                             buf_samps / in_chans, in_chans);
      if (status == -1)
         exit(EXIT_FAILURE);
#endif
#ifdef MACOSX
      macosx_cmixplay_audio_write(sbuf);
#endif
   }

   /* read input samples until end_frame, and copy to audio port,
      byte-swapping and converting from floats as necessary.
      If not running in quiet mode, print seconds as they elapse.
   */
   buf_start_frame = start_frame;
   for (  ; buf_start_frame < end_frame; buf_start_frame += nframes) {
      int  samps_read;
      long bytes_read;
      char c = 0;

      if (buf_start_frame + buf_frames > end_frame) {      /* last buffer */
         int samps = (end_frame - buf_start_frame) * in_chans;
         bytes_read = read(fd, bufp, samps * datum_size);
      }
      else
         bytes_read = read(fd, bufp, buf_samps * datum_size);
      if (bytes_read == -1) {
         perror("read");
#ifdef LINUX
         close_ports(afd, in_chans);
#endif
#ifdef MACOSX
#endif
         exit(EXIT_FAILURE);
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
      else if (is_24bit) {
         int j;
         if (data_format == MUS_L24INT) {
            for (i = j = 0; i < samps_read; i++, j += datum_size) {
               float samp = (float) (((cbuf[j + 2] << 24)
                                    + (cbuf[j + 1] << 16)
                                    + (cbuf[j] << 8)) >> 8);
               samp *= factor;
               sbuf[i] = (short) (samp / (float) (1 << 8));
            }
         }
         else {   /* data_format == MUS_B24INT */
            for (i = j = 0; i < samps_read; i++, j += datum_size) {
               float samp = (float) (((cbuf[j] << 24)
                                    + (cbuf[j + 1] << 16)
                                    + (cbuf[j + 2] << 8)) >> 8);
               samp *= factor;
               sbuf[i] = (short) (samp / (float) (1 << 8));
            }
         }
      }
      else {   /* 16bit int */
         if (swap) {
            if (factor) {
               for (i = 0; i < samps_read; i++) {
                  sbuf[i] = reverse_int2(&sbuf[i]);
                  sbuf[i] = (int) (factor * sbuf[i]);
               }
            }
            else {
               for (i = 0; i < samps_read; i++)
                  sbuf[i] = reverse_int2(&sbuf[i]);
            }
         }
         else {
            if (factor)
               sbuf[i] = (int) (factor * sbuf[i]);
         }
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

#ifdef LINUX
      status = write_buffer(afd, (char *)sbuf, AUDIO_DATUM_SIZE, nframes,
                                                                  in_chans);
      if (status == -1)
         exit(EXIT_FAILURE);
#endif
#ifdef MACOSX
      macosx_cmixplay_audio_write(sbuf);
#endif

      if (!quiet) {
         if (buf_start_time >= second) {
            if (print_minutes_seconds)
               printf("%s ", make_time_string((float)second, 0));
            else
               printf("%d ", second);
            fflush(stdout);
            second++;
         }
      }

      /* Handle hotkeys */
      if (hotkeys) {
         bytes_read = read(STDIN_FILENO, &c, 1);
         if (bytes_read) {
            int   skip = 0;

            if (c == '\004')        /* control-D */
               break;
            else if (c == 'f') {    /* fast-forward */
               skip = 1;
               skip_bytes = (long)(skip_frames * in_chans * datum_size);
               buf_start_frame += skip_frames;
               if (!quiet) {
                  buf_start_time += skip_time;
                  second += (int)skip_time;
                  printf("\n");
               }
            }
            else if (c == 'r') {    /* rewind */
               skip = 1;
               skip_bytes = -(long)(skip_frames * in_chans * datum_size);
               buf_start_frame -= skip_frames;
               if (!quiet) {
                  buf_start_time -= skip_time;
                  second -= (int)skip_time;
                  printf("\n");
               }
            }
            else if (c == 'm') {    /* mark */
               float mark_time = buf_start_time - MARK_DELAY;
               printf("\nMARK: %.*f [%s]\n", MARK_PRECISION, mark_time,
                                 make_time_string(mark_time, MARK_PRECISION));
               fflush(stdout);
            }
            else if (c == 't')
               print_minutes_seconds = print_minutes_seconds ? 0 : 1;

            if (skip) {
               off_t curloc = lseek(fd, 0, SEEK_CUR);
               if (curloc + skip_bytes <= data_location) {
                  if (!quiet) {
                     buf_start_time = 0.0;
                     second = 0;
                  }
                  if (lseek(fd, data_location, SEEK_SET) == -1) {
                     perror("lseek");
                     exit(EXIT_FAILURE);
                  } 
                  buf_start_frame = -nframes;
               }
               else if (lseek(fd, skip_bytes, SEEK_CUR) == -1) {
                  perror("lseek");
                  exit(EXIT_FAILURE);
               }
            }
         }
      }

      buf_start_time += (float)nframes / (float)srate;
   }

   /* write buffers of zeros to prevent clicks */
   for (i = 0; i < buf_samps; i++)
      sbuf[i] = 0;
   for (i = 0; i < ZERO_FRAMES_AFTER / buf_frames; i++) {
#ifdef LINUX
      status = write_buffer(afd, (char *)sbuf, AUDIO_DATUM_SIZE, nframes,
                                                                  in_chans);
      if (status == -1)
         exit(EXIT_FAILURE);
#endif
#ifdef MACOSX
      macosx_cmixplay_audio_write(sbuf);
#endif
   }

#ifdef LINUX
   close_ports(afd, in_chans);
#endif
#ifdef MACOSX
#endif
   close(fd);

   if (!quiet)
      printf("\n");

   return EXIT_SUCCESS;
}


#endif /* #if defined(LINUX) || defined(MACOSX) */
