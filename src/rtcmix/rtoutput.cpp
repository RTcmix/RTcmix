/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sndlibsupport.h>
#include "../rtstuff/rtdefs.h"

static int clobber = 0;         /* Default clobber mode (see comment below) */


/* ------------------------------------------------- set_rtoutput_clobber --- */
void
set_rtoutput_clobber(int state)
{
   clobber = state;
}


/* The syntax of rtoutput is expanded when using sndlib:

   rtoutput("filename" [, "header_type"] [, "data_format"])

   - "header_type" is one of:
        "aiff", "aifc", "wav", "next", "sun", or "ircam"

      The default is "aiff", since this is what most other unix
      programs can use (Notam software, Cecilia, etc.).

      "next" and "sun" (the familiar ".au" format) are synonyms.
      "ircam" is the older, non-hybrid BICSF format.
      (Note that sndlib can't write the hybrid EBICSF headers.)

      All formats are bigendian, except for "wav".

   - "data_format" is one of:
        "short"      16-bit linear
        "float"      32-bit floating point
        "normfloat"  32-bit floating point in range (mostly) from -1.0 to +1.0
        "16"         synonym for "short"
        "24"         24-bit linear, not yet supported in RTcmix

        The default is "short".


   NOTES:

     (1) The sampling rate and number of channels are specified in
         the call to rtsetparams.

     (2) If you ask for "aiff" and "float" (or "normfloat"),
         you'll get "aifc" format instead. This is because sndlib
         doesn't handle floats properly with AIFF format.

     (3) The case of the "header_type" and "data_format" arguments
         is not significant, nor is their order.

     (4) If you want to use floating point files in Snd, choose
         "normfloat" format. If you want to use them in Mxv, choose
         the "next" header type. Many programs don't read AIFC files,
         maybe because they assume these are always compressed.

   There are other possibilities with sndlib, but limiting the choices
   to those listed above makes for an easier Minc interface to rtoutput.
*/


/* CAUTION: Don't change these without thinking about constraints
            imposed by the various formats, and by sndlib and any
            programs (Mxv, Mix, Cecilia) that should be compatible.
*/
#define DEFAULT_HEADER_TYPE    AIFF_sound_file
#define DEFAULT_DATA_FORMAT    snd_16_linear

int output_header_type = -1;
int output_data_format = -1;
int is_float_format = 0;
int normalize_output_floats = 0;
char *rtoutsfname;

#define CLOBBER_WARNING       \
"Specified output file already exists! \n\n\
Turn on \"clobber mode\" in your score to overwrite it.\n\
(Put \"set_option(\"CLOBBER_ON\")\" before call to rtoutput).\n"


typedef enum {
   INVALID_PARAM = 0,
   HEADER_TYPE,
   DATA_FORMAT,
   ENDIANNESS
} ParamType;

typedef struct {
   ParamType  type;
   int        value;
   char       arg[16];
} Param;

/* See description of these strings above.
   (Supporting endian request would be really confusing, because there
   are many constraints in the formats. The only thing it would buy
   us is the ability to specify little-endian AIFC linear formats.
   Not worth the trouble.)
*/
static Param param_list[] = {
   { HEADER_TYPE,  NeXT_sound_file,  "next"      },
   { HEADER_TYPE,  NeXT_sound_file,  "sun"       },
   { HEADER_TYPE,  AIFF_sound_file,  "aiff"      },
   { HEADER_TYPE,  AIFF_sound_file,  "aifc"      },
   { HEADER_TYPE,  RIFF_sound_file,  "wav"       },
   { HEADER_TYPE,  IRCAM_sound_file, "ircam"     },
   { DATA_FORMAT,  snd_16_linear,    "short"     },
   { DATA_FORMAT,  snd_32_float,     "float"     },
   { DATA_FORMAT,  snd_32_float,     "normfloat" },
   { DATA_FORMAT,  snd_16_linear,    "16"        },
   { DATA_FORMAT,  snd_24_linear,    "24"        },    /* not yet supported */
   { ENDIANNESS,   0,                "big"       },    /* not implemented */
   { ENDIANNESS,   1,                "little"    }
};
static int num_params = sizeof(param_list) / sizeof(Param);

static int parse_rtoutput_args(int, double []);


/* -------------------------------------------------- parse_rtoutput_args --- */
static int
parse_rtoutput_args(int nargs, double pp[])
{
   int   anint, i, j, matched;
   int   aifc_requested, normfloat_requested;
   char  *arg;

   if (nargs == 0) {
      fprintf(stderr, "rtoutput: you didn't specify a file name!\n");
      return -1;
   }

   /* This is the ancient method of casting a double to a char ptr. */
   anint = (int)pp[0];
   rtoutsfname = (char *)anint;

   output_header_type = DEFAULT_HEADER_TYPE;
   output_data_format = DEFAULT_DATA_FORMAT;

   aifc_requested = normfloat_requested = 0;

   for (i = 1; i < nargs; i++) {
      anint = (int)pp[i];
      arg = (char *)anint;

      matched = 0;
      for (j = 0; j < num_params; j++) {
         if (strcasecmp(param_list[j].arg, arg) == 0) {
            matched = 1;
            break;
         }
      }
      if (!matched) {
         fprintf(stderr, "rtoutput: unrecognized argument \"%s\"\n", arg);
         return -1;
      }

      switch (param_list[j].type) {
         case HEADER_TYPE:
            output_header_type = param_list[j].value;
            if (output_header_type == AIFF_sound_file
                                && strcasecmp(param_list[j].arg, "aifc") == 0)
               aifc_requested = 1;
            break;
         case DATA_FORMAT:
            output_data_format = param_list[j].value;
            if (output_data_format == snd_32_float
                           && strcasecmp(param_list[j].arg, "normfloat") == 0)
               normfloat_requested = 1;
            break;
         case ENDIANNESS:  /* currently unused */
            break;
         default:
            break;
      }
   }

   /* Handle some special cases. */

   /* If "wav", make data format little-endian. */
   if (output_header_type == RIFF_sound_file) {
      switch (output_data_format) {
         case snd_16_linear:
            output_data_format = snd_16_linear_little_endian;
            break;
         case snd_24_linear:
            output_data_format = snd_24_linear_little_endian;
            break;
         case snd_32_float:
            output_data_format = snd_32_float_little_endian;
            break;
      }
   }

   /* If AIFF, use AIFC only if explicitely requested, or if
      the data format is float.
   */
   if (output_header_type == AIFF_sound_file) {
      if (output_data_format == snd_32_float)
         aifc_requested = 1;
      set_aifc_header(aifc_requested);      /* in sndlib/headers.c */
   }

   /* If writing to a float file, decide whether to normalize the
      samples, i.e., to divide them all by 32768 so as to make the
      normal range fall between -1.0 and +1.0. This is what Snd
      and sndlib like to see, but it's not the old cmix way.
   */
   if (normfloat_requested)
      normalize_output_floats = 1;

   is_float_format = IS_FLOAT_FORMAT(output_data_format);

#ifdef ALLBUG
   fprintf(stderr, "name: %s, head: %d, data: %d, aifc: %d, norm: %d\n",
                   rtoutsfname, output_header_type, output_data_format,
                   aifc_requested, normalize_output_floats);
#endif

   return 0;
}


/* ------------------------------------------------------------- rtoutput --- */
/* This routine is used in the Minc score to open up a file for
   writing by RT instruments.  pp[0] is a pointer to the soundfile
   name, disguised as a double by the crafty Minc.  (p[] is passed in
   just for fun.)  Optional string arguments follow the filename,
   and parse_rtoutput_args processes these. See the comment at the
   top of this file for the meaning of these arguments.

   If "clobber" mode is on, we delete an existing file with the
   specified file name, creating a header according to what
   parse_rtoutput_args determines.

   Returns -1.0 if a file is already open for writing. Dies if there
   is any other error.
*/
double
rtoutput(float p[], int n_args, double pp[])
{
   int         error;
   struct stat statbuf;

   if (rtfileit == 1) {
      fprintf(stderr, "A soundfile is already open for writing...\n");
      return -1.0;
   }

   error = parse_rtoutput_args(n_args, pp);
   if (error)
      exit(1);          /* already reported in parse_rtoutput_args */

   error = stat(rtoutsfname, &statbuf);

   if (error) {
      if (errno == ENOENT) { 
         ;              /* File doesn't exist -- no problem */
      }
      else {
         fprintf(stderr, "Error accessing file \"%s\": %s\n",
                                                rtoutsfname, strerror(errno));
         exit(1);
      }
   }
   else {               /* File exists; find out whether we can clobber it */
      if (!clobber) {
         fprintf(stderr, "\n%s\n", CLOBBER_WARNING);
         exit(1);
      }
      else {
         /* make sure it's a regular file */
         if (!S_ISREG(statbuf.st_mode)) {
            fprintf(stderr, "\"%s\" isn't a regular file; won't clobber it\n",
                                                                 rtoutsfname);
            exit(1);
         }

         /* try to delete it */
         error = unlink(rtoutsfname);
         if (error) {
            fprintf(stderr, "Error deleting clobbered file \"%s\" (%s)\n",
                                                rtoutsfname, strerror(errno));
            exit(1);
         }
      }
   }

   rtoutfile = sndlib_create(rtoutsfname, output_header_type,
                                         output_data_format, (int)SR, NCHANS);
   if (rtoutfile == -1) {
      fprintf(stderr, "Can't write \"%s\" (%s)\n",
                                                rtoutsfname, strerror(errno));
      exit(1);
   }

   if (print_is_on) {
     printf("Output file set for writing:\n");
     printf("      name:  %s\n", rtoutsfname);
     printf("      type:  %s\n", sound_type_name(output_header_type));
     printf("    format:  %s\n", sound_format_name(output_data_format));
     printf("     srate:  %g\n", SR);
     printf("     chans:  %d\n", NCHANS);
   }

   rtfileit = 1;

   return 1.0;
}


