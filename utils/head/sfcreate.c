#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sndlibsupport.h>

/* #define DEBUG */

#ifndef TRUE
 #define TRUE (1)
#endif
#ifndef FALSE
 #define FALSE (0)
#endif

enum {
   UNSPECIFIED_ENDIAN = 0,
   CREATE_BIG_ENDIAN,
   CREATE_LITTLE_ENDIAN
};

#define DEFAULT_SRATE          44100
#define DEFAULT_NCHANS         2
#define DEFAULT_ENDIAN         CREATE_BIG_ENDIAN
#define DEFAULT_IS_SHORT       TRUE
#define DEFAULT_FORMAT_NAME    "aiff"

#define MINSRATE               11025
#define MAXSRATE               96000

#define USAGE_MSG  "\
  option     description                    default                     \n\
  --------------------------------------------------------------------- \n\
  -r NUM     sampling rate                  [44100]                     \n\
  -c NUM     number of channels             [2]                         \n\
  -i or -f   16-bit integer or 32-bit float [16-bit integer]            \n\
  -b or -l   big-endian or little-endian    [LE for wav, BE for others] \n\
  -t NAME    file format name; one of...    [aiff, aifc for float]      \n\
             aiff, aifc, wav, next, sun, ircam                          \n\
             (sun is a synonym for next)                                \n\
                                                                        \n\
  Defaults take effect unless overridden by supplied values.            \n\
                                                                        \n\
  If filename exists, its header will be overwritten with the one       \n\
  specified.  If filename doesn't already have a valid header, or       \n\
  if overwriting an existing header might result in some loss or        \n\
  corruption of sound data, the program will warn about this and        \n\
  require you to use the \"--force\" flag on the command line.          \n\
                                                                        \n\
  NOTE: The following combinations are not available:                   \n\
        aiff and -l                                                     \n\
        aiff and -f (will substitute aifc here)                         \n\
        aifc and -f and -l together (aifc floats are BE only)           \n\
        ircam and -l                                                    \n\
        next and -l                                                     \n\
        wav and -b                                                      \n\
"


#define OVERWRITE_WARNING                                                 \
"You're asking to change the header type or sound data format of an     \n\
existing file. Changing the header type could result in good samples    \n\
deleted from the beginning of the file (or garbage samples added there).\n\
Also, channels could be swapped; sample words could even be corrupted.  \n\
Changing the data format specified in the header would cause a program  \n\
to interpret the existing sound data in a new way, probably as loud,    \n\
painful noise. If you still want to write over the header, run the      \n\
program with the \"--force\" option."

#define WORD_INTEGRITY_WARNING                                            \
"WARNING: Sample words have been corrupted!"

#define CHANNEL_SWAP_WARNING                                              \
"WARNING: Channels have been swapped!"


char *progname, *sfname = NULL;
int srate = DEFAULT_SRATE;
int nchans = DEFAULT_NCHANS;
int is_short = DEFAULT_IS_SHORT;
int endian = UNSPECIFIED_ENDIAN;
char comment[DEFAULT_COMMENT_LENGTH] = "";

#define FORMAT_NAME_LENGTH 32
char format_name[FORMAT_NAME_LENGTH] = DEFAULT_FORMAT_NAME;

/* these are assigned in check_params */
static int header_type;
static int data_format;

static void usage(void);
static int check_params(void);


/* ---------------------------------------------------------------- usage --- */
static void
usage()
{
   printf("\nusage: \"%s [options] filename\"\n\n", progname);
   printf("%s", USAGE_MSG);
   exit(1);
}


/* --------------------------------------------------------- check_params --- */
/* Convert user specifications into a form we can hand off to sndlib.
   Validate input, and make sure the combination of parameters specifies
   a valid sound file format type (i.e., one whose header sndlib can write).
   See the usage message for the types we allow.
*/
static int
check_params()
{
   if (nchans < 1 || nchans > MAXCHANS) {
      fprintf(stderr, "Number of channels must be between 1 and %d.\n\n",
              MAXCHANS);
      exit(1);
   }
   if (srate < MINSRATE || srate > MAXSRATE) {
      fprintf(stderr, "Sampling rate must be between %d and %d.\n\n",
              MINSRATE, MAXSRATE);
      exit(1);
   }

   if (strcasecmp(format_name, "aiff") == 0)
      header_type = MUS_AIFF;
   else if (strcasecmp(format_name, "aifc") == 0)
      header_type = MUS_AIFC;
   else if (strcasecmp(format_name, "wav") == 0)
      header_type = MUS_RIFF;
   else if (strcasecmp(format_name, "next") == 0)
      header_type = MUS_NEXT;
   else if (strcasecmp(format_name, "sun") == 0)
      header_type = MUS_NEXT;
   else if (strcasecmp(format_name, "ircam") == 0)
      header_type = MUS_IRCAM;
   else {
      fprintf(stderr, "Invalid file format name: %s\n", format_name);
      fprintf(stderr, "Valid names: aiff, aifc, wav, next, sun, ircam\n");
      exit(1);
   }

   if (endian == UNSPECIFIED_ENDIAN) {
      if (header_type == MUS_RIFF)
         endian = CREATE_LITTLE_ENDIAN;
      else
         endian = DEFAULT_ENDIAN;
   }

   if (endian == CREATE_BIG_ENDIAN)
      data_format = is_short? MUS_BSHORT : MUS_BFLOAT;
   else
      data_format = is_short? MUS_LSHORT : MUS_LFLOAT;

   /* Check for combinations of parameters we don't allow.
      See the NOTE at bottom of usage message.
   */
   if (header_type == MUS_AIFF) {
      if (endian == CREATE_LITTLE_ENDIAN) {
         fprintf(stderr, "AIFF little-endian header not allowed.\n");
         exit(1);
      }
      if (!is_short) {
         header_type = MUS_AIFC;
         fprintf(stderr, "WARNING: using AIFC header for floats.\n");
      }
   }
   if (header_type == MUS_AIFC) {
      if (data_format == MUS_LFLOAT) {
         fprintf(stderr, "AIFC little-endian float header not allowed.\n");
         exit(1);
      }
   }
   if (header_type == MUS_IRCAM && endian == CREATE_LITTLE_ENDIAN) {
      fprintf(stderr, "IRCAM little-endian header not allowed.\n");
      exit(1);
   }
   if (header_type == MUS_NEXT && endian == CREATE_LITTLE_ENDIAN) {
      fprintf(stderr, "NeXT little-endian header not allowed.\n");
      exit(1);
   }
   if (header_type == MUS_RIFF && endian == CREATE_BIG_ENDIAN) {
      fprintf(stderr, "RIFF big-endian header not allowed.\n");
      exit(1);
   }

#ifdef DEBUG
   printf("%s: nchans=%d, srate=%d, fmtname=%s, %s, %s\n",
          sfname, nchans, srate, format_name,
          is_short? "short" : "float",
          endian? "bigendian" : "littleendian");
#endif

   return 0;
}


/* ----------------------------------------------------------------- main --- */
/* Create a soundfile header of the specified type. The usage msg above
   describes the kind of types you can create. (These are ones that
   sndlib can write and that can be useful to the average cmix user.
   There are other kinds of header sndlib can write -- for example,
   for 24-bit files -- but we want to keep the sfcreate syntax as
   simple as possible.

   We allocate a generous comment area, because expanding it after a
   sound has already been written would mean copying the entire file.
   (We use comments to store peak stats -- see sys/sndlibsupport.c.)
*/
int
main(int argc, char *argv[])
{
   int         i, fd, result, overwrite_file, old_format, old_header_type=0;
   int         data_location, old_data_location, old_nsamps, old_datum_size;
   int         force = FALSE;
   struct stat statbuf;

   /* get name of this program */
   progname = strrchr(argv[0], '/');
   if (progname == NULL)
      progname = argv[0];
   else
      progname++;

   if (argc < 2)
      usage();

   for (i = 1; i < argc; i++) {
      char *arg = argv[i];

      if (arg[0] == '-') {
         switch (arg[1]) {
            case 'c':
               if (++i >= argc)
                  usage();
               nchans = atoi(argv[i]);
               break;
            case 'r':
               if (++i >= argc)
                  usage();
               srate = atoi(argv[i]);
               break;
            case 'i':
               is_short = TRUE;
               break;
            case 'f':
               is_short = FALSE;
               break;
            case 'b':
               endian = CREATE_BIG_ENDIAN;
               break;
            case 'l':
               endian = CREATE_LITTLE_ENDIAN;
               break;
            case 't':
               if (++i >= argc)
                  usage();
               strncpy(format_name, argv[i], FORMAT_NAME_LENGTH - 1);
               format_name[FORMAT_NAME_LENGTH - 1] = 0; /* ensure termination */
               break;
            case '-':
               if (strcmp(arg, "--force") == 0)
                  force = TRUE;
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
   if (sfname == NULL)
      usage();

   if (check_params())
      usage();

   old_data_location = old_format = old_datum_size = old_nsamps = 0;

   /* Test for existing file. If there is one, and we can read and write it,
      and the force flag is set, then we'll overwrite its header.  Note that
      we slap on a header even if the file doesn't have one already, as 
      would be the case with a raw sound file (or a precious text file...).
      If there isn't an existing file, we'll create a new one.
   */
   overwrite_file = FALSE;
   result = stat(sfname, &statbuf);
   if (result == -1) {
      if (errno == ENOENT) {       /* file doesn't exist, we'll create one */
         fd = open(sfname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
         if (fd == -1) {
            fprintf(stderr, "Error creating file \"%s\" (%s)\n",
                                                      sfname, strerror(errno));
            exit(1);
         }
      }
      else {
         fprintf(stderr,
              "File \"%s\" exists, but there was an error accessing it (%s)\n",
                                                      sfname, strerror(errno));
         exit(1);
      }
   }
   else {
      int drastic_change;

      overwrite_file = TRUE;

      /* File exists and we could stat it. If it's a regular file, open
         it and see if it looks like a sound file.
      */
      if (!S_ISREG(statbuf.st_mode)) {
         fprintf(stderr, "\"%s\" exists, but it's not a regular file.\n",
                                                                      sfname);
         exit(1);
      }
      fd = open(sfname, O_RDWR);
      if (fd == -1) {
         fprintf(stderr, "Error opening file (%s)\n", strerror(errno));
         exit(1);
      }
      if (sndlib_read_header(fd) == -1) {
         fprintf(stderr, "Error reading header (%s)\n", strerror(errno));
         exit(1);
      }
      old_header_type = mus_header_type();
      old_format = mus_header_format();
      if (NOT_A_SOUND_FILE(old_header_type)
                                         || INVALID_DATA_FORMAT(old_format)) {
         fprintf(stderr, "\nWARNING: \"%s\" exists, but doesn't look like "
                         "a sound file.\n\n", sfname);
         drastic_change = TRUE;
         old_header_type = MUS_RAW;
      }
      else if (old_header_type != header_type || old_format != data_format)
         drastic_change = TRUE;
      else
         drastic_change = FALSE;

      if (drastic_change && !force) {
         fprintf(stderr, "%s\n", OVERWRITE_WARNING);
         exit(1);
      }

      old_data_location = mus_header_data_location();
      old_nsamps = mus_header_samples();         /* samples, not frames */
      old_datum_size = mus_header_data_format_to_bytes_per_sample();
   }

   result = sndlib_write_header(fd, 0, header_type, data_format, srate, nchans,
                                                        NULL, &data_location);
   if (result == -1) {
      fprintf(stderr, "Error writing header (%s)\n", strerror(errno));
      exit(1);
   }

   /* If we're overwriting a header, we have to fiddle around a bit
      to get the correct data_size into the header.
      (These will not likely be the same if we're changing header types.)
   */
   if (overwrite_file) {
      int loc_byte_diff, datum_size, sound_bytes;

      /* need to do this again */
      if (sndlib_read_header(fd) == -1) {
         fprintf(stderr, "Error re-reading header (%s)\n", strerror(errno));
         exit(1);
      }

      if (data_format != old_format && old_header_type != MUS_RAW)
        if (! FORMATS_SAME_BYTE_ORDER(data_format, old_format))
            printf("WARNING: Byte order changed!\n");

      datum_size = mus_header_data_format_to_bytes_per_sample();

      loc_byte_diff = data_location - old_data_location;

      /* If the data locations have changed, we're effectively adding or
         subtracting sound data bytes. Depending on the number of bytes
         added, this could result in swapped channels or worse. We can't
         do anything about this, because our scheme for encoding peak stats
         in the header comment requires a fixed comment allocation in the
         header. (Otherwise, we could shrink or expand this to make things
         right.) The best we can do is warn the user.
      */
      if (loc_byte_diff) {
         if (loc_byte_diff > 0)
            printf("Losing %d bytes of sound data\n", loc_byte_diff);
         else if (loc_byte_diff < 0)
            printf("Gaining %d bytes of sound data\n", -loc_byte_diff);

         if (loc_byte_diff % datum_size)
            printf("%s\n", WORD_INTEGRITY_WARNING);
         else if (loc_byte_diff % (nchans * datum_size))
            printf("%s\n", CHANNEL_SWAP_WARNING);
         /* else got lucky: no shifted words or swapping */

         sound_bytes = (old_nsamps * old_datum_size) - loc_byte_diff;  
      }
      else                               /* same number of bytes as before */
         sound_bytes = old_nsamps * old_datum_size;  

      result = sndlib_set_header_data_size(fd, header_type, sound_bytes);
      if (result == -1) {
         fprintf(stderr, "Error updating header\n");
         exit(1);
      }

      close(fd);
   }
   else
      close(fd);

   return 0;
}

