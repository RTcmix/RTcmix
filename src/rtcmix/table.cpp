/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifdef PFIELD_CLASS
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <rtcmix_types.h>
#include <prototypes.h>
#include <byte_routines.h>
#include <sndlibsupport.h>
#include <PField.h>
#include <ugens.h>      /* for warn, die */

/* Functions for creating and modifying double arrays.  These can be passed
   from a script to RTcmix functions that can accept them.  Much of this code
   is derived from the old RTcmix makegens.

   John Gibson and Doug Scott, May/June 2004
*/

#define MAX_ARRAY_LEN 1000000  /* guard against unintentionally huge allocs */

#ifndef M_PI
  #define M_PI    3.14159265358979323846
#endif
#ifndef TWOPI
  #define TWOPI   M_PI * 2
#endif


typedef enum {
   InvalidTable = -1,
   TextfileTable = 0,
   SndfileTable = 1,
   LiteralTable = 2,
   DatafileTable = 3,
   CurveTable = 4,
   ExpbrkTable = 5,
   LineTable = 6,
   LinebrkTable = 7,
   SplineTable = 8,
   Wave3Table = 9,
   WaveTable = 10,
   ChebyTable = 17,
   RandomTable = 20,
   WindowTable = 25,
   EndTable
} TableKind;

static char *_table_name[] = {
   "textfile", /* 0 */
   "sndfile",
   "literal",
   "datafile",
   "curve",    /* 4 */
   "expbrk",
   "line",
   "linebrk",
   "spline",   /* 8 */
   "wave3",
   "wave",
   NULL,
   NULL,       /* 12 */
   NULL,
   NULL,
   NULL,
   NULL,       /* 16 */
   "cheby",
   "line",
   NULL,
   "random",   /* 20 */
   NULL,
   NULL,
   NULL,
   "line",     /* 24 */
   "window",
   NULL
};

inline int min(int x, int y) { return (x <= y) ? x : y; }


/* ------------------------------------------------------- local utilities -- */
static TablePField *
_getTablePField(const Arg *arg)
{
   PField *tpf = NULL;
   if ((tpf = (PField *) *arg) != NULL) {
#if __GNUG__ >= 3
      return dynamic_cast<TablePField *> (tpf);
#else
      return (TablePField *) tpf;
#endif
   }
   return NULL;
}

Handle
_createPFieldHandle(PField *pfield)
{
   Handle handle = (Handle) malloc(sizeof(struct _handle));
   handle->type = PFieldType;
   handle->ptr = (void *) pfield;
   return handle;
}


/* --------------------------------------------------- args_have_same_type -- */
// FIXME: this belongs in a more general file
int
args_have_same_type(const Arg args[], const int nargs, const RTcmixType type)
{
   for (int i = 0; i < nargs; i++)
      if (!args[i].isType(type))
         return 0;
   return 1;
}


/* ------------------------------------------------------ _normalize_table -- */
/* Rescale the values in <array> so that the maximum absolute value is <peak>,
   which cannot be zero.  Similar to cmix fnscl.
*/
static void
_normalize_table(double *array, const int len, const double peak)
{
   int i;
   double max = 0.0;

   assert(peak != 0.0);

   for (i = 0; i < len; i++) {
      double absval = fabs(array[i]);
      if (absval > max)
         max = absval;
   }
   max /= peak;
   if (max != 0.0)
      for (i = 0; i < len; i++)
         array[i] /= max;
}


/* ------------------------------------------------------- _textfile_table -- */
/* Fill a table with numbers read from a text file.  The syntax is

      table = maketable("textfile", size, filename)

   The function loads as many as <size> numbers into the table.  If there
   are not that many numbers in the text file, it zeros out the extra
   table values.  The function reports one warning if at least one piece
   of text (delimited by whitespace) cannot be interpreted as a double.
   This means the file may contain any number of free text comments
   interspersed with the numbers, as long as the comments themselves
   do not contain numbers!

   Similar to gen 2, except it supports only the text file input, it takes
   a file name instead of a cmix file number, and it does not return the
   number of elements in the array.  Get this with tablelen().
                                                           - JGG, 6/20/04
*/
static int
_textfile_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (textfile)", "Table length must be at least 2.");
   if (nargs <= 0)
      return die("maketable (textfile)",
                 "Usage: table = maketable(\"textfile\", size, filename)");

   if (!args[0].isType(StringType))
      return die("maketable (textfile)", "File name must be a string.");
   const char *fname = (const char *) args[0];

   FILE *stream = fopen(fname, "r");
   if (stream == NULL)
      return die("maketable (textfile)", "Can't open file \"%s\".", fname);

   bool non_convertible_found = false;
   int i;
   for (i = 0; i < len; i++) {
      char buf[128 + 1];
      if (fscanf(stream, "%128s", buf) == EOF)
         break;
      char *pos = NULL;
      double val = strtod(buf, &pos);
      if (val == 0.0 && pos == buf) {  /* no conversion performed; warn once */
         if (!non_convertible_found)
            warn("maketable (textfile)",
                "File contains some text that can't be converted to a number.");
         non_convertible_found = true;
         i--;
         continue;
      }
      if (errno == ERANGE)             /* overflow or underflow */
         return die("maketable (textfile)",
                    "A number in the file can't be represented as a double.");
      array[i] = val;
   }

   float bogusval;
   if (fscanf(stream, "%f", &bogusval) != EOF)
      warn("maketable (textfile)", "File \"%s\" has more than %d numbers.",
                                                                  fname, len);
   fclose(stream);

   if (i != len) {
      warn("maketable (textfile)", "Only %d values loaded into table, "
                                   "followed by %d padding zeros.", i, len - i);
      /* Fill remainder with zeros. */
      for ( ; i < len; i++)
         array[i] = 0.0;
   }
   else
      advise("maketable (textfile)", "%d values loaded into table.", i);

   return 0;
}


/* -------------------------------------------------------- _sndfile_table -- */
/* Fill a table with numbers read from a sound file.  The syntax is

   table = maketable("sndfile", size, filename[, duration[, inskip[, inchan]]])

   The <size> argument is ignored; set it to zero.

   <filename> is obligatory.  The other arguments are optional, but if an
   argument further to the right is given, all the ones to its left must also
   be given.

      filename    name of a sound file in any of the header types RTcmix can
                  read; data formats: 16bit signed int, 32bit float, 24bit
                  3-byte signed int; either endian.

      duration    duration (in seconds) to read from file.  If negative, its
                  absolute value is the number of sample frames to read.

                  If <duration> is missing, or if it's zero, then the whole
                  file is read.  Beware with large files -- there is no check
                  on memory consumption!

      inskip      time (in seconds) to skip before reading, or if negative,
                  its absolute value is the number of sample frames to skip.

                  If <inskip> is missing, it's assumed to be zero.

      inchan      channel number to read (with zero as first channel).

                  If <inchan> is missing all channels are read, with samples
                  from each frame interleaved.

   NOTE: We dispose the array passed in and allocate a new one to replace it.
         This is because we can't know how big the array must be before we
         look at the sound file header.

   Similar to gen 1, but no normalization.  The arguments are different also,
   and it does not return the number of frames.  Get this with tablelen().
                                                   - JGG 2/7/01, rev. 6/19/04
*/

#define ALL_CHANS -1
#define BUFSAMPS  1024 * 16

static int
_sndfile_table(const Arg args[], const int nargs, double **array, int *len)
{
   delete [] *array;    /* need to allocate our own */

   if (nargs <= 0)
      return die("maketable (sndfile)",
                 "\nUsage: table = maketable(\"sndfile\", size=0, "
                                 "filename[, duration[, inskip[, inchan]]])");

   if (!args[0].isType(StringType))
      return die("maketable (sndfile)", "File name must be a string.");
   const char *fname = (const char *) args[0];

   double request_dur = 0.0;
   double inskip = 0.0;
   int inchan = ALL_CHANS;
   if (nargs > 1) {
      if (!args[1].isType(DoubleType))
         return die("maketable (sndfile)", "<duration> must be a number.");
      request_dur = args[1];
      if (nargs > 2) {
         if (!args[2].isType(DoubleType))
            return die("maketable (sndfile)", "<inskip> must be a number.");
         inskip = args[2];
         if (nargs > 3) {
            if (!args[3].isType(DoubleType))
               return die("maketable (sndfile)", "<inchan> must be a number.");
            inchan = args[3];
         }
      }
   }

   int header_type, data_format, data_location, file_chans;
   long file_samps;
   double srate;

   int fd = open_sound_file((char *) fname, &header_type, &data_format,
                        &data_location, &srate, &file_chans, &file_samps);
   if (fd == -1)
      return die("maketable (sndfile)", "Can't open input file \"%s\".", fname);

   if (srate != SR)
      warn("maketable (sndfile)", "The input file sampling rate is %g, but "
           "the output rate is currently %g.", srate, SR);

   int file_frames = file_samps / file_chans;

   int table_chans = file_chans;
   if (inchan != ALL_CHANS) {
      if (inchan >= file_chans)
         return die("maketable (sndfile)",
                    "You asked for channel %d of a %d-channel file. (\"%s\")",
                    inchan, file_chans, fname);
      table_chans = 1;
   }

   int table_frames = file_frames;        /* if request_dur == 0 */
   if (request_dur < 0.0)
      table_frames = (int) -request_dur;
   else if (request_dur > 0.0)
      table_frames = (int) (request_dur * srate + 0.5);

   int start_frame = (int) (inskip * srate + 0.5); 
   if (inskip < 0.0)
      start_frame = (int) -inskip;

   if (start_frame + table_frames > file_frames)
      table_frames = file_frames - start_frame;

   int table_samps = table_frames * table_chans;
 
   double *block = new double[table_samps];
   if (block == NULL)
      return die("maketable (sndfile)", "Not enough memory for table.");

   int bytes_per_samp = mus_data_format_to_bytes_per_sample(data_format);

   char *buf = new char[BUFSAMPS * bytes_per_samp];
   if (buf == NULL)
      return die("maketable (sndfile)",
                                    "Not enough memory for temporary buffer.");

   off_t seek_to = data_location + (start_frame * file_chans * bytes_per_samp);
   if (lseek(fd, seek_to, SEEK_SET) == -1)
      return die("maketable (sndfile)", "File seek error: %s", strerror(errno));

#if MUS_LITTLE_ENDIAN
   bool byteswap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
   bool byteswap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
   bool is_float = IS_FLOAT_FORMAT(data_format);
   bool is_24bit = IS_24BIT_FORMAT(data_format);

   int buf_frames = BUFSAMPS / file_chans;
   int end_frame = start_frame + table_frames;

   double *blockp = block;
   int frames_read = 0;
   int buf_start_frame = start_frame;
   for ( ; buf_start_frame < end_frame; buf_start_frame += frames_read) {
      long bytes_read;

      if (buf_start_frame + buf_frames > end_frame) {      /* last buffer */
         int samps = (end_frame - buf_start_frame) * file_chans;
         bytes_read = read(fd, buf, samps * bytes_per_samp);
      }
      else
         bytes_read = read(fd, buf, BUFSAMPS * bytes_per_samp);
      if (bytes_read == -1)
         return die("maketable (sndfile)", "File read error: %s",
                                                            strerror(errno));
      if (bytes_read == 0)          /* EOF, somehow */
         break;

      int samps_read = bytes_read / bytes_per_samp;
      frames_read = samps_read / file_chans;

      int increment = 1;
      if (is_float) {
         float *bufp = (float *) buf;

         if (inchan != ALL_CHANS) {
            bufp += inchan;
            increment = file_chans;
         }
         if (byteswap) {
            for (int i = 0; i < samps_read; i += increment) {
               byte_reverse4(bufp);          /* modify *bufp in place */
               *blockp++ = (double) *bufp;
               bufp += increment;
            }
         }
         else {
            for (int i = 0; i < samps_read; i += increment) {
               *blockp++ = (double) *bufp;
               bufp += increment;
            }
         }
      }
      if (is_24bit) {
         unsigned char *bufp = (unsigned char *) buf;

         if (inchan != ALL_CHANS) {
            bufp += inchan * 3;              /* 3-byte samples */
            increment = file_chans;
         }
         if (data_format == MUS_L24INT) {
            for (int i = 0; i < samps_read; i += increment) {
               int samp = (int) (((bufp[2] << 24)
                                + (bufp[1] << 16)
                                + (bufp[0] << 8)) >> 8);
               *blockp++ = (double) samp / (double) (1 << 8);
               bufp += increment * 3;
            }
         }
         else {   /* data_format == MUS_B24INT */
            for (int i = 0; i < samps_read; i += increment) {
               int samp = (int) (((bufp[0] << 24)
                                + (bufp[1] << 16)
                                + (bufp[2] << 8)) >> 8);
               *blockp++ = (double) samp / (double) (1 << 8);
               bufp += increment * 3;
            }
         }
      }
      else {     /* is 16bit integer file */
         short *bufp = (short *) buf;

         if (inchan != ALL_CHANS) {
            bufp += inchan;
            increment = file_chans;
         }
         if (byteswap) {
            for (int i = 0; i < samps_read; i += increment) {
               short samp = reverse_int2(bufp);
               *blockp++ = (double) samp;
               bufp += increment;
            }
         }
         else {
            for (int i = 0; i < samps_read; i += increment) {
               *blockp++ = (double) *bufp;
               bufp += increment;
            }
         }
      }
   }

   delete [] buf;
   sndlib_close(fd, 0, 0, 0, 0);

   *array = block;
   *len = table_samps;

   return 0;
}


/* -------------------------------------------------------- _literal_table -- */
/* Fill a table with just the values specified as arguments.  The syntax is

      table = maketable("literal", "nonorm", size, a, b, c, d ...)

   <a>, <b>, <c>, etc. are the numbers that go into the table.  The
   "nonorm" tag is recommended, unless you want the numbers to be
   normalized to [-1,1] or [0,1].

   The function loads as many as <size> numbers into the table.  If there
   are not that many number arguments, it zeros out the extra table values.
   If <size> is zero, the table will be sized to fit the number arguments
   exactly.

   Similar to gen 2, except it supports only the "new way" described in
   gen2.c, it has the option to size the table to fit the number of arguments,
   and it does not return the number of elements in the array.  Get this
   with tablelen().
                                                           - JGG, 6/20/04
*/
static int
_literal_table(const Arg args[], const int nargs, double **array, int *len)
{
   double *block = *array;
   int length = *len;

   if (length == 0) {
      length = nargs;
      delete [] block;
      block = new double[length];
   }

   const int n = min(nargs, length);

   for (int i = 0; i < n; i++)
      block[i] = args[i];

   if (nargs < length) {
      for (int i = nargs; i < length; i++)
         block[i] = 0.0;
      advise("maketable (literal)",
                           "Table is larger than the number of elements given "
                           "to fill it.  Adding zeros to pad.");
   }
   else if (length < nargs)
      warn("maketable (literal)",
                        "Table is large enough for only %d numbers.", length);

   *array = block;
   *len = length;

   return 0;
}


/* ------------------------------------------------------- _datafile_table -- */
/* Fill a table with numbers read from a data file.  The syntax is

      table = maketable("datafile", size, filename[, number_type])

   The function loads as many as <size> numbers into the table.  If there
   are not that many numbers in the file, it zeros out the extra table values.
   If <size> is zero, the table will be sized to fit the data exactly.  Be
   careful if you use this option with a very large file: you may run out
   of memory!

   <number_type> can be any of "float" (the default), "double", "int",
   "short" or "byte".  This just means that with the "double" type, for
   example, every 8 bytes will be interpreted as one floating point number;
   with the "int" type, every 4 bytes will be interpreted as one integer;
   and so on.

   Similar to gen 3, except that it accepts a file name rather than a cmix
   file number, it has choices for the type of number, it has the option to
   size the table to fit the data, and it does not return the number of
   elements in the array.  Get this with tablelen().
                                                           - JGG, 6/20/04
*/
static int
_datafile_table(const Arg args[], const int nargs, double **array, int *len)
{
   double *block = *array;
   int length = *len;
   long cur;

   if (nargs <= 0)
      return die("maketable (datafile)",
         "Usage: table = maketable(\"datafile\", size, filename, number_type)");

   if (!args[0].isType(StringType))
      return die("maketable (datafile)", "File name must be a string.");
   const char *fname = (const char *) args[0];

   size_t size = sizeof(float);     /* default */
   if (nargs > 1) {
      if (!args[1].isType(StringType))
         return die("maketable (datafile)", "File number type must be a string "
                    "\"double\", \"float\", \"int\", \"short\" or \"byte\"");
      if (args[1] == "double")
         size = sizeof(double);
      else if (args[1] == "float")
         size = sizeof(float);
      else if (args[1] == "int")
         size = sizeof(int);
      else if (args[1] == "short")
         size = sizeof(short);
      else if (args[1] == "byte")
         size = sizeof(char);
      else
         return die("maketable (datafile)", "File number type must be "
                    "\"double\", \"float\", \"int\", \"short\" or \"byte\"");
   }

   FILE *stream = fopen(fname, "r");
   if (stream == NULL)
      return die("maketable (datafile)", "Can't open file \"%s\".", fname);

   if (length == 0) {
      if (fseek(stream, 0, SEEK_END) != 0)
         return die("maketable (datafile)", "File seek error: %s",
                                                            strerror(errno));
      length = ftell(stream) / size;
      rewind(stream);
      delete [] block;
      block = new double[length];
   }

   /* Read data file until EOF or table is filled, using specified word size. */
   int i;
   if (size == sizeof(float)) {
      float val;
      for (i = 0; i < length; i++) {
         if (fread(&val, size, 1, stream) < 1) {
            if (ferror(stream))
               goto readerr;
            break;
         }
         block[i] = (double) val;
      }
   }
   else if (size == sizeof(double)) {
      double val;
      for (i = 0; i < length; i++) {
         if (fread(&val, size, 1, stream) < 1) {
            if (ferror(stream))
               goto readerr;
            break;
         }
         block[i] = val;
      }
   }
   else if (size == sizeof(int)) {
      int val;
      for (i = 0; i < length; i++) {
         if (fread(&val, size, 1, stream) < 1) {
            if (ferror(stream))
               goto readerr;
            break;
         }
         block[i] = (double) val;
      }
   }
   else if (size == sizeof(short)) {
      short val;
      for (i = 0; i < length; i++) {
         if (fread(&val, size, 1, stream) < 1) {
            if (ferror(stream))
               goto readerr;
            break;
         }
         block[i] = (double) val;
      }
   }
   else {  /* byte */
      char val;
      for (i = 0; i < length; i++) {
         if (fread(&val, size, 1, stream) < 1) {
            if (ferror(stream))
               goto readerr;
            break;
         }
         block[i] = (double) val;
      }
   }

   cur = ftell(stream);
   if (fseek(stream, 0, SEEK_END) != 0)
      return die("maketable (datafile)", "File seek error: %s",
                                                            strerror(errno));
   if (ftell(stream) != cur)
      warn("maketable (datafile)", "File \"%s\" has more than %d numbers.",
                                                               fname, length);
   fclose(stream);

   if (i != length) {
      warn("maketable (datafile)", "Only %d values loaded into table, "
                                "followed by %d padding zeros.", i, length - i);
      /* Fill remainder with zeros. */
      for ( ; i < length; i++)
         block[i] = 0.0;
   }
   else
      advise("maketable (datafile)", "%d values loaded into table.", i);

   *array = block;
   *len = length;

   return 0;
readerr:
   return die("maketable (datafile)", "Error reading file \"%s\".", fname);
}


/* ----------------------------------------------- _curve_table and helper -- */
/* Fill a table with line or curve segments, defined by
   <time, value, curvature> arguments.  The syntax is...

      table = maketable("curve", size, time1, value1, curvature1, [ timeN-1,
                                       valueN-1, curvatureN-1, ] timeN, valueN)

   <curvature> controls the curvature of the line segment from point N
   to point N+1.  The values are

      curvature = 0
                  makes a straight line

      curvature < 0
                  makes a logarithmic (convex) curve

      curvature > 0
                  makes a exponential (concave) curve

   Derived from gen4 from the UCSD Carl package, described in F.R. Moore, 
   "Elements of Computer Music."
                                                   -JGG, 12/2/01, rev 1/25/04
*/

/* _transition(a, alpha, b, n, output) makes a transition from <a> to <b> in
   <n> steps, according to transition parameter <alpha>.  It stores the
   resulting <n> values starting at location <output>.
      alpha = 0 yields a straight line,
      alpha < 0 yields an logarithmic transition, and 
      alpha > 0 yields a exponential transition.
   All of this in accord with the formula:
      output[i] = a + (b - a) * (1 - exp(i * alpha / (n-1))) / (1 - exp(alpha))
   for 0 <= i < n
*/
static void
_transition(double a, double alpha, double b, int n, double *output)
{
   int    i;
   double delta, interval = 0.0;

   delta = b - a;

   if (n <= 1) {
      warn("maketable (curve)", "Trying to transition over 1 array slot; "
                                "time between points is too short");
      *output = a;
      return;
   }
   interval = 1.0 / (n - 1.0);

   if (alpha != 0.0) {
      double denom = 1.0 / (1.0 - exp(alpha));
      for (i = 0; i < n; i++)
         *output++ = (a + delta
                        * (1.0 - exp((double) i * alpha * interval)) * denom);
   }
   else
      for (i = 0; i < n; i++)
         *output++ = a + delta * i * interval;
}

#define MAX_CURVE_PTS 256

static int
_curve_table(const Arg args[], const int nargs, double *array, const int len)
{
   int    i, points, seglen = 0;
   double factor, *ptr;
   double time[MAX_CURVE_PTS], value[MAX_CURVE_PTS], alpha[MAX_CURVE_PTS];

   if (len < 2)
      return die("maketable (curve)", "Table length must be at least 2.");
   if (nargs < 5 || (nargs % 3) != 2)        /* check number of args */
      return die("maketable (curve)", "\nUsage: table = maketable(\"curve\", "
                 "size, time1, value1, curvature1, [ timeN-1, valueN-1, "
                 "curvatureN-1, ] timeN, valueN)");
   if ((nargs / 3) + 1 > MAX_CURVE_PTS)
      return die("maketable (curve)", "Too many arguments.");
   if (!args_have_same_type(args, nargs, DoubleType))
      return die("maketable (curve)",
                           "<time, value, curvature> pairs must be numbers.");
   if ((double) args[0] != 0.0)
      return die("maketable (curve)", "First time must be zero.");

   for (i = points = 0; i < nargs; points++) {
      time[points] = (double) args[i++];
      if (points > 0 && time[points] < time[points - 1])
         goto time_err;
      value[points] = (double) args[i++];
      if (i < nargs)
         alpha[points] = (double) args[i++];
   }

   factor = (double) (len - 1) / time[points - 1];
   for (i = 0; i < points; i++)
      time[i] *= factor;

   ptr = array;
   for (i = 0; i < points - 1; i++) {
      seglen = (int) (floor(time[i + 1] + 0.5) - floor(time[i] + 0.5)) + 1;
      _transition(value[i], alpha[i], value[i + 1], seglen, ptr);
      ptr += seglen - 1;
   }

   return 0;
time_err:
   return die("maketable (curve)", "Times must be in ascending order.");
}


/* --------------------------------------------------------- _expbrk_table -- */
/* Similar to gen 5, but no normalization.
*/
static int
_expbrk_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (expbrk)", "Table length must be at least 2.");
   double amp2 = args[0];
   if (amp2 <= 0.0)
      amp2 = 0.00001;
   int i = 0;
   for (int k = 1; k < nargs; k += 2) {
      double amp1 = amp2;
      amp2 = args[k + 1];
      if (amp2 <= 0.0)
         amp2 = 0.00001;
      int j = i + 1;
      array[i] = amp1;
      double c = pow((amp2 / amp1), (1.0 / (double) args[k]));
      i = (j - 1) + (int) args[k];
      for (int l = j; l < i; l++) {
         if (l < len)
            array[l] = array[l - 1] * c;
      }
   }

   return 0;
}


/* ----------------------------------------------------------- _line_table -- */
/* Create a function table comprising straight line segments, specified by
   using any number of <time, value> pairs.  The table is not normalized.

   Derived from cmix gen 24, but no table normalization, and it spreads values
   evenly across the array in such a way that the last array element has the
   same value as the last argument to maketable.  (None of the cmix setline
   workalikes -- gen6, gen18, gen24 -- do this unless they normalize the table
   result with fnscl.)  This helps you avoid clicks when using as an amp
   envelope and ramping to zero at the end.  The gens never get to zero.  On
   the other hand, this function gets to zero too early, so it's important to
   use tables large enough that the last array slot controls a very short
   amount of time.  (I.e., total time / array len should be just a few msecs
   for critical situations.)  This new behavior is turned on by defining
   NEWWAY symbol below.  This function also works when the first time is
   non-zero.  (This seems to work in gen24, but not in gen6 and gen18.)

   -JGG, 1/25/04
*/
#define NEWWAY
static int
_line_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (line)", "Table length must be at least 2.");
   if (!args_have_same_type(args, nargs, DoubleType))
      return die("maketable (line)", "<time, value> pairs must be numbers.");
   if ((nargs % 2) != 0)
      return die("maketable (line)", "Incomplete <time, value> pair.");

   double endtime = (double) args[nargs - 2];
   double starttime = (double) args[0];
   if (endtime - starttime <= 0.0)
      return die("maketable (line)", "Times must be in ascending order.");
#ifdef NEWWAY
   double scaler = (double) (len - 1) / (endtime - starttime);
#else
   double scaler = (double) len / (endtime - starttime);
#endif
   double nextval = (double) args[1];
   double thistime = starttime;
   int i = 0;
   for (int k = 1; k < nargs; k += 2) {
      double nexttime;
      double thisval = nextval;
      if (k < nargs - 1) {
         nexttime = (double) args[k + 1] - starttime;
         if (nexttime - thistime < 0.0)   /* okay for them to be the same */
            return die("maketable (line)", "Times must be in ascending order.");
         nextval = (double) args[k + 2];
      }
      else
         nextval = nexttime = 0.0;        /* don't read off end of args array */
      int j = i + 1;
#ifdef NEWWAY
      i = (int) ((nexttime * scaler) + 0.5);
#else
      i = (int) ((nexttime * scaler) + 1.0);
#endif
      //printf("j=%d, i=%d\n", j, i);
      for (int l = j; l <= i; l++) {
         if (l <= len)
            array[l - 1] = thisval + (nextval - thisval)
                                          * (double) (l - j) / ((i - j) + 1);
      }
   }
#ifdef NEWWAY
   array[len - 1] = (double) args[nargs - 1];
#endif

   return 0;
}


/* -------------------------------------------------------- _linebrk_table -- */
/* Similar to gen 7, but no normalization.
*/
static int
_linebrk_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (linebrk)", "Table length must be at least 2.");

   double amp2 = args[0];
   int i = 0;
   for (int k = 1; k < nargs; k += 2) {
      double amp1 = amp2;
      amp2 = args[k + 1];
      int j = i + 1;
      i = j + (int) args[k] - 1;
      for (int l = j; l <= i; l++) {
         if (l <= len)
            array[l - 1] = amp1
                           + (amp2 - amp1) * (double) (l - j) / (i - j + 1);
      }
   }

   return 0;
}


/* --------------------------------------------- _spline_table and helpers -- */
/* Fill a table with a spline curve, defined by at least three x,y points.
   The curve travels smoothly between the points, and all points lie on the
   curve.  The syntax is

      table = maketable("spline", size, ["closed",] curvature,
                                          time1, value1, ... timeN, valueN)

   <curvature> controls the character of the slope between points.  Start
   with zero; try positive numbers up to around 200 to see the difference.

   The option "closed" tag is another way of affecting the curvature.
   You just have to experiment with "closed" and <curvature> to get a
   feel for what they do to a particular shape.

   It is possible that the curve will loop outside of the area you expect,
   especially with the "nonorm" tag, so use plottable to be sure.

   Adapted from cspline from the UCSD Carl package, described in F.R. Moore, 
   "Elements of Computer Music."
                                                           - JGG, 6/20/04
*/

#define MAX_KNOTS 1024

typedef struct {
   bool manual_lb, manual_ub;
   float lower_bound, upper_bound, val[MAX_KNOTS];
} SplineSpec;


static void
_spline_interp(const float *inbuf, const int count, double *outbuf,
                                                      const int outbuflen)
{
   float rat = count * 1.0 / (float) outbuflen;
   int c = 0;
   float fc = 0.0;
   for (int i = outbuflen - 1; i >= 0; fc += rat, i--) {
      c = (int) fc;
      float frat = fc - (float) c;        /* get fraction */
      outbuf[i] = (double) ((1.0 - frat) * inbuf[c] + frat * inbuf[c + 1]);
   }
}


inline float
_rhs(const int knot, const int nknots, const SplineSpec *x, const SplineSpec *y)
{
   int i = (knot == nknots - 1) ? 0 : knot;
   double zz = (y->val[knot] - y->val[knot - 1])
                                    / (x->val[knot] - x->val[knot - 1]);
   return (6.0 * ((y->val[i + 1] - y->val[i])
                                    / (x->val[knot + 1] - x->val[knot]) - zz));
}


static int
_spline(const int closed, const float konst, const int nknots, double *outbuf,
        const int outbuflen, const SplineSpec *x, const SplineSpec *y)
{
   int count = 0;
   int buflen = BUFSIZ;
   float *buf = (float *) malloc(buflen * sizeof(float));  /* realloc'd below */
   if (buf == NULL)
      return die("maketable (spline)", "Out of memory.");

   float *diag = new float[nknots + 1];
   float *r = new float[nknots + 1];

   r[0] = 0.0;

   float a = 0.0;
   float d = 1.0;
   float u = 0.0;
   float v = 0.0;
   float s = closed ? -1.0 : 0.0;
   for (int i = 0; ++i < nknots - !closed; ) {        /* triangularize */
      float hi = x->val[i] - x->val[i - 1];
      float hi1 = (i == nknots - 1) ? x->val[1] - x->val[0]
                               : x->val[i + 1] - x->val[i];
      if (hi1 * hi <= 0)
         return -1;
      u = i == 1 ? 0.0 : u - s * s / d;
      v = i == 1 ? 0.0 : v - s * r[i - 1] / d;
      r[i] = _rhs(i, nknots, x, y) - hi * r[i - 1] / d;
      s = -hi * s / d;
      a = 2.0 * (hi + hi1);
      if (i == 1)
         a += konst * hi;
      if (i == nknots - 2)
         a += konst * hi1;
      diag[i] = d = (i == 1) ? a : a - hi * hi / d;
   }
   float D2yi = 0.0;
   float D2yn1 = 0.0;
   for (int i = nknots - !closed; --i >= 0; ) {       /* back substitute */
      int end = i == nknots - 1;
      float hi1 = end ? x->val[1] - x->val[0] : x->val[i + 1] - x->val[i];
      float D2yi1 = D2yi;
      if (i > 0) {
         float hi = x->val[i] - x->val[i - 1];
         float corr = end ? 2.0 * s + u : 0.0;
         D2yi = (end * v + r[i] - hi1 * D2yi1 - s * D2yn1) /
             (diag[i] + corr);
         if (end)
            D2yn1 = D2yi;
         if (i > 1) {
            a = 2 * (hi + hi1);
            if (i == 1)
               a += konst * hi;
            if (i == nknots - 2)
               a += konst * hi1;
            d = diag[i - 1];
            s = -s * d / hi;
         }
      }
      else
         D2yi = D2yn1;
      if (!closed) {
         if (i == 0)
            D2yi = konst * D2yi1;
         if (i == nknots - 2)
            D2yi1 = konst * D2yi;
      }
      if (end)
         continue;
      int m = (hi1 > 0.0) ? outbuflen : -outbuflen;
      m = (int) (1.001 * m * hi1 / (x->upper_bound - x->lower_bound));
      if (m <= 0)
         m = 1;
      float h = hi1 / m;
      for (int j = m; j > 0 || i == 0 && j == 0; j--) {
         /* interpolate */
         float x0 = (m - j) * h / hi1;
         float x1 = j * h / hi1;
         float yy = D2yi * (x0 - x0 * x0 * x0) + D2yi1 * (x1 - x1 * x1 * x1);
         yy = y->val[i] * x0 + y->val[i + 1] * x1 - hi1 * hi1 * yy / 6.0;

         if (count < buflen)
            buf[count++] = yy;
         else {
            buflen += BUFSIZ;
            buf = (float *) realloc(buf, buflen * sizeof(float));
            if (buf == NULL)
               return die("maketable (spline)", "Out of memory.");
            buf[count++] = yy;
         }
      }
   }

   delete [] diag;
   delete [] r;

   _spline_interp(buf, count, outbuf, outbuflen);

   free(buf);

   return 0;
}


static void
_spline_getlimit(SplineSpec *p, int nknots)
{
   for (int i = 0; i < nknots; i++) {
      if (!p->manual_lb && p->lower_bound > p->val[i])
         p->lower_bound = p->val[i];
      if (!p->manual_ub && p->upper_bound < p->val[i])
         p->upper_bound = p->val[i];
   }
}


#define INF 1.e37

static int
_spline_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (spline)", "Table length must be at least 2.");

   int closed = 0;
   float curvature = 0.0;

   int firstknot = 1;
   if (args[0].isType(StringType)) {
      if (args[0] == "closed")
         closed = 1;
      firstknot++;
   }
   else
      curvature = (float) args[0];

   if (((nargs - firstknot) % 2) != 0)
      return die("maketable (spline)", "Incomplete <time, value> pair.");
   if ((nargs - firstknot) < 6)
      return die("maketable (spline)", "Need at least 3 <time, value> pairs.");
   if ((nargs - firstknot) > MAX_KNOTS * 2)
      return die("maketable (spline)", "Too many <time, value> pairs.");

   SplineSpec x, y;

   x.manual_lb = x.manual_ub = y.manual_lb = y.manual_ub = 0;
   x.lower_bound = y.lower_bound = INF;
   x.upper_bound = y.upper_bound = -INF;
   /* It's possible to manually set the lower and upper bounds here
      (set the manual_lb and manual_ub flags as well), but it didn't
      seem worth supporting this.   -JGG
   */

   float last_timeval = -INF;
   int nknots = 0;
   for (int i = firstknot; i < nargs; i += 2) {
      float this_timeval = args[i];
      x.val[nknots] = this_timeval;
      y.val[nknots] = args[i + 1];
      if (this_timeval <= last_timeval)
         return die("maketable (spline)", "Times must be ascending.");
      last_timeval = this_timeval;
      nknots++;
   }

   _spline_getlimit(&x, nknots);
   //_spline_getlimit(&y, nknots);   JGG: y bounds not used

   if (_spline(closed, curvature, nknots, array, len, &x, &y) != 0)
      return -1;

   return 0;
}


/* ---------------------------------------------------------- _wave3_table -- */
/* Similar to cmix gen 9, but no normalization.
*/
static int
_wave3_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (wave3)", "Table length must be at least 2.");
   if ((nargs % 3) != 0)
      return die("maketable (wave3)",
                                 "Incomplete <partial, amp, phase> triplet.");

   for (int i = 0; i < len; i++)
      array[i] = 0.0;

   for (int j = nargs - 1; j > 0; j -= 3) {
      assert(j > 0);
      assert(j < nargs);
      if ((double) args[j - 1] != 0.0) {
         for (int i = 0; i < len; i++) {
            double val = sin(TWOPI * ((double) i
                                 / ((double) len / (double) args[j - 2])
                                 + (double) args[j] / 360.0));
            array[i] += (val * (double) args[j - 1]);
         }
      }
   }

   return 0;
}


/* ----------------------------------------------------------- _wave_table -- */
/* Equivalent to cmix gen 10.
*/
static int
_wave_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (wave)", "Table length must be at least 2.");

   for (int i = 0; i < len; i++)
      array[i] = 0.0;
   int j = nargs;
   while (j--) {
      if (!args[j].isType(DoubleType))
         return die("maketable (wave)", "Harmonic amplitudes must be numbers.");
      if ((double) args[j] != 0.0) {
         for (int i = 0; i < len; i++) {
            double val = TWOPI * (double) i / (len / (j + 1));
            array[i] += (sin(val) * (double) args[j]);
         }
      }
   }

   return 0;
}


/* ---------------------------------------------------------- _cheby_table -- */
/* Equivalent to cmix gen 17.  Computes transfer function using Chebyshev
   polynomials.  First argument is the index value for which the function will
   create the harmonics specified by the following arguments.
*/
static int
_cheby_table(const Arg args[], const int nargs, double *array, const int len)
{
   if (len < 2)
      return die("maketable (cheby)", "Table length must be at least 2.");

   double d = (double) ((len / 2) - 0.5);
   for (int i = 0; i < len; i++) {
      double x = (i / d - 1.0) / (double) args[0];
      array[i] = 0.0;
      double Tn1 = 1.0;
      double Tn = x;
      for (int j = 1; j < nargs; j++) {
         array[i] += (double) args[j] * Tn;
         double Tn2 = Tn1;
         Tn1 = Tn;
         Tn = (2.0 * x * Tn1) - Tn2;
      }
   }

   return 0;
}


/* --------------------------------------------------------- _random_table -- */
/* Same as gen 20, the original version of which was written by Luke Dubois;
   later additions by John Gibson.

   The arguments are...

      table = maketable("random", size, type[, seed[, min, max]])

   The distribution types are:
     0 = even distribution ["even"]
     1 = low weighted linear distribution ["low"]
     2 = high weighted linear distribution ["high"]
     3 = triangle linear distribution ["triangle"]
     4 = gaussian distribution ["gaussian"]
     5 = cauchy distribution ["cauchy"]
    
   (Distribution equations adapted from Dodge and Jerse.)

   If <seed> is zero, seed comes from microsecond clock, otherwise <seed>
   is used as the seed.  If no <seed> argument, the seed used is 1.

   <min> and <max> define the range (inclusive) for the random numbers.
   Both args must be present; otherwise the range is from 0 to 1.
*/

/* Scale <num>, which falls in range [0,1] so that it falls
   in range [min,max].  Return result.    -JGG, 12/4/01
*/
static inline double
fit_range(double min, double max, double num)
{
   return min + (num * (max - min));
}

static inline double
_brrand(long *randx)
{
   *randx = (*randx * 1103515245) + 12345;
   int k = (*randx >> 16) & 077777;
   return (double) k / 32768.0;
}

static int
_random_table(const Arg args[], const int nargs, double *array, const int len)
{
   static long randx = 1;
   int type;

   if (len < 2)
      return die("maketable (random)", "Table length must be at least 2.");

   if (args[0].isType(StringType)) {
      if (args[0] == "even")
         type = 0;
      else if (args[0] == "low")
         type = 1;
      else if (args[0] == "high")
         type = 2;
      else if (args[0] == "triangle")
         type = 3;
      else if (args[0] == "gaussian")
         type = 4;
      else if (args[0] == "cauchy")
         type = 5;
      else
         return die("maketable (random)",
                    "Unsupported distribution type \"%s\".",
                    (const char *) args[0]);
   }
   else if (args[0].isType(DoubleType))
      type = (int) args[0];
   else
      return die("maketable (random)",
                              "Distribution type must be string or number.");

   if ((int) args[1] == 0) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      randx = tv.tv_usec;
   }
   else
      randx = (int) args[1];

   /* Set range for random numbers. */
   if (nargs == 3)
      return die("maketable (random)",
              "Usage: maketable(\"random\", size, type[, seed[, min, max]])");

   double min, max;
   if (nargs == 4) {
      min = args[2];
      max = args[3];
      if (min == max)
         return die("maketable (random)", "<min> must be lower than <max>.");
      if (min > max) {     /* make sure these are in increasing order */
         double tmp = max;
         max = min;
         min = tmp;
      }
   }
   else {
      min = 0.0;
      max = 1.0;
   }

   switch (type) {
      case 0:  /* even distribution */
         for (int i = 0; i < len; i++) {
            double tmp = _brrand(&randx);
            array[i] = fit_range(min, max, tmp);
         }
         break;
      case 1:  /* low weighted */
         for (int i = 0; i < len; i++) {
            double randnum = _brrand(&randx);
            double randnum2 = _brrand(&randx);
            if (randnum2 < randnum)
               randnum = randnum2;
            array[i] = fit_range(min, max, randnum);
         }
         break;
      case 2:  /* high weighted */
         for (int i = 0; i < len; i++) {
            double randnum = _brrand(&randx);
            double randnum2 = _brrand(&randx);
            if (randnum2 > randnum)
               randnum = randnum2;
            array[i] = fit_range(min, max, randnum);
         }
         break;
      case 3:  /* triangle */
         for (int i = 0; i < len; i++) {
            double randnum = _brrand(&randx);
            double randnum2 = _brrand(&randx);
            double tmp = 0.5 * (randnum + randnum2);
            array[i] = fit_range(min, max, tmp);
         }
         break;
      case 4:  /* gaussian */
         {
            const int N = 12;
            const double halfN = 6.0;
            const double scale = 1.0;
            const double mu = 0.5;
            const double sigma = 0.166666;
            int i = 0;
            while (i < len) {
               double randnum = 0.0;
               for (int j = 0; j < N; j++)
                  randnum += _brrand(&randx);
               double output = sigma * scale * (randnum - halfN) + mu;
               if (output <= 1.0 && output >= 0.0) {
                  array[i] = fit_range(min, max, output);
                  i++;
               }
            }
         }
         break;
      case 5:  /* cauchy */
         {
            const double alpha = 0.00628338;
            int i = 0;
            while (i < len) {
               double randnum = 0.0;
               do {
                  randnum = _brrand(&randx);
               } while (randnum == 0.5);
               randnum *= PI;
               double output = (alpha * tan(randnum)) + 0.5;
               if (output <= 1.0 && output >= 0.0) {
                  array[i] = fit_range(min, max, output);
                  i++;
               }
            }
         }
         break;
      default:
         return die("maketable (random)", "Unsupported distribution type %d.",
                                                                        type);
         break;
   }

   return 0;
}


/* --------------------------------------------------------- _window_table -- */
/* Similar to gen 25, but no normalization.
*/
static int
_window_table(const Arg args[], const int nargs, double *array, const int len)
{
   int window_type = 0;

   if (len < 2)
      return die("maketable (window)", "Table length must be at least 2.");
   if (nargs != 1)
      return die("maketable (window)", "Missing window type.");

   if (args[0].isType(StringType)) {
      if (args[0] == "hanning")
         window_type = 1;
      else if (args[0] == "hamming")
         window_type = 2;
      else
         return die("maketable (window)", "Unsupported window type \"%s\".",
                                                      (const char *) args[0]);
   }
   else if (args[0].isType(DoubleType)) {
        window_type = (int) args[0];
   }
   else
      return die("maketable (window)",
                              "Window type must be string or numeric code.");

   switch (window_type) {
      case 1:     /* hanning window */
         for (int i = 0; i < len; i++)
            array[i] = -cos(2.0 * M_PI * (double) i / (double) len) * 0.5 + 0.5;
         break;
      case 2:     /* hamming window */
         for (int i = 0; i < len; i++) {
            double val = cos(2.0 * M_PI * (double) i / (double) len);
            array[i] = 0.54 - 0.46 * val;
         }
         break;
      default:
         return die("maketable (window)", "Unsupported window type (%d).",
                                                               window_type);
   }

   return 0;
}


/* ------------------------------------------------------- _dispatch_table -- */

static TableKind
_string_to_tablekind(const char *str)
{
   int i;

   for (i = 0; i < (int) EndTable; i++) {
      if (_table_name[i] == NULL)
         continue;
      if (strncmp(str, _table_name[i], 16) == 0)
         return (TableKind) i;
   }
   return InvalidTable;
}

static int
_dispatch_table(const Arg args[], const int nargs, const int startarg,
   double **array, int *len)
{
   int status;
   TableKind tablekind;

   /* Call the appropriate factory function, skipping over first two args. */
   if (args[0].isType(DoubleType))
      tablekind = (TableKind) (int) args[0];
   else if (args[0].isType(StringType)) {
      tablekind = _string_to_tablekind((const char *) args[0]);
      if (tablekind == InvalidTable)
         return die("maketable", "Invalid table type string \"%s\"",
                                                      (const char *) args[0]);
   }
   else
      return die("maketable", "First argument must be a number or string.");

   /* NOTE: passing addresses of array and len is correct for some of these
            tables, because they might need to recreate their array.
   */
   switch (tablekind) {
      case TextfileTable:
         status = _textfile_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      case SndfileTable:
         status = _sndfile_table(&args[startarg], nargs - startarg, array, len);
         break;
      case LiteralTable:
         status = _literal_table(&args[startarg], nargs - startarg, array, len);
         break;
      case DatafileTable:
         status = _datafile_table(&args[startarg], nargs - startarg, array,
                                                                        len);
         break;
      case CurveTable:
         status = _curve_table(&args[startarg], nargs - startarg, *array, *len);
         break;
      case ExpbrkTable:
         status = _expbrk_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      case LineTable:
         status = _line_table(&args[startarg], nargs - startarg, *array, *len);
         break;
      case LinebrkTable:
         status = _linebrk_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      case SplineTable:
         status = _spline_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      case Wave3Table:
         status = _wave3_table(&args[startarg], nargs - startarg, *array, *len);
         break;
      case WaveTable:
         status = _wave_table(&args[startarg], nargs - startarg, *array, *len);
         break;
      case ChebyTable:
         status = _cheby_table(&args[startarg], nargs - startarg, *array, *len);
         break;
      case RandomTable:
         status = _random_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      case WindowTable:
         status = _window_table(&args[startarg], nargs - startarg, *array,
                                                                        *len);
         break;
      default:
         return die("maketable", "Invalid table type.");
         break;
   }

   return status;
}


/* ========================================================================== */
/* The remaining functions are public, callable from scripts. */

extern "C" {
   Handle maketable(const Arg args[], const int nargs);
   double tablelen(const Arg args[], const int nargs);
   Handle normtable(const Arg args[], const int nargs);
   Handle copytable(const Arg args[], const int nargs);
   Handle multtable(const Arg args[], const int nargs);
   Handle addtable(const Arg args[], const int nargs);
   double plottable(const Arg args[], const int nargs);
   double dumptable(const Arg args[], const int nargs);
};

/* ------------------------------------------------------------- maketable -- */
static void
_maketable_usage()
{
   die("maketable",
      "\n    usage: table = maketable(type, [option, ] length, ...)\n");
}

Handle
maketable(const Arg args[], const int nargs)
{
   int status, lenindex;
   bool normalize = true;
   double *data;
   Handle handle;

   if (nargs < 2) {
      _maketable_usage();
      return NULL;
   }
   if (args[1].isType(StringType)) {
      if (args[1] == "nonorm")
         normalize = false;
      else if (args[1] == "norm")
         normalize = true;
      else {
         die("maketable", "Invalid string option \"%s\".",
                                                      (const char *) args[1]);
         return NULL;
      }
      lenindex = 2;
   }
   else
      lenindex = 1;
   if (!args[lenindex].isType(DoubleType)) {
      _maketable_usage();
      return NULL;
   }
   int len = args[lenindex];
   if (len < 0) {    // NOTE: It's okay for len to be zero (cf _sndfile_table)
      die("maketable", "Negative table size.");
      return NULL;
   }
   if (len > MAX_ARRAY_LEN) {
      warn("maketable", "Requesting larger than maximum table length.  "
                                          "Setting to %d.", MAX_ARRAY_LEN);
      len = MAX_ARRAY_LEN;
   }

   // Allocate table array.  TablePField will own and delete this.
   
   data = new double[len];
   if (data == NULL) {
      die("maketable", "Out of memory.");
      return NULL;
   }

   status = _dispatch_table(args, nargs, lenindex + 1, &data, &len);

   if (normalize)
      _normalize_table(data, len, 1.0);

   return _createPFieldHandle(new TablePField(data, len));
}

/* -------------------------------------------------------------- tablelen -- */
double
tablelen(const Arg args[], const int nargs)
{
   double len;

   if (nargs != 1)
      return die("tablelen", "Takes only one argument: a valid table handle.");
   if (!args[0].isType(HandleType))
      return die("tablelen", "Argument must be a valid table handle.");

   TablePField *table = _getTablePField(&args[0]);
   if (table == NULL)
      return die("tablelen", "Argument must be a valid table handle.");

   return (double) table->values();
}

/* ------------------------------------------------------------- multtable -- */
Handle
multtable(const Arg args[], const int nargs)
{
   if (nargs == 2) {
      PField *table0 = (PField *) args[0];
      PField *table1 = (PField *) args[1];
      if (!table1) {
         if (args[1].isType(DoubleType)) {
            table1 = new ConstPField((double) args[1]);
         }
      }
      else if (!table0) {
         if (args[0].isType(DoubleType)) {
            table0 = new ConstPField((double) args[0]);
         }
      }
      if (table0 && table1) {
         return _createPFieldHandle(new MultPField(table0, table1));
      }
   }
   die("multtable", "Usage: mul(table1, table2) or mul(table1, const1)");
   return NULL;
}

/* -------------------------------------------------------------- addtable -- */
Handle
addtable(const Arg args[], const int nargs)
{
   if (nargs == 2) {
      PField *table0 = (PField *) args[0];
      PField *table1 = (PField *) args[1];
      if (!table1) {
         if (args[1].isType(DoubleType)) {
            table1 = new ConstPField((double) args[1]);
         }
      }
      else if (!table0) {
         if (args[0].isType(DoubleType)) {
            table0 = new ConstPField((double) args[0]);
         }
      }
      if (table0 && table1) {
         return _createPFieldHandle(new AddPField(table0, table1));
      }
   }
   die("addtable", "Usage: add(table1, table2) or add(table1, const1)");
   return NULL;
}

/* ------------------------------------------------------------- normtable -- */
Handle
normtable(const Arg args[], const int nargs)
{
   double peak = 1.0;

   if (nargs < 1) {
      die("normtable", "Requires at least one argument (table to normalize).");
      return NULL;
   }
   TablePField *table = _getTablePField(&args[0]);
   if (table == NULL) {
      die("normtable",
          "First argument must be a handle to the table to normalize.");
      return NULL;
   }
   if (nargs > 1 && args[1].isType(DoubleType))
      peak = (double) args[1];

   double *values = new double[table->values()];
   table->copyValues(values);
   _normalize_table(values, table->values(), peak);
   TablePField *newtable = new TablePField(values, table->values());

   return _createPFieldHandle(newtable);
}

/* ------------------------------------------------------------- copytable -- */
Handle
copytable(const Arg args[], const int nargs)
{
   if (nargs != 1) {
      die("copytable", "Usage: newtable = copytable(table_to_copy);");
      return NULL;
   }
   PField *table = (PField *) args[0], *copyOfTable = NULL;
   if (table == NULL) {
      die("copytable", "Usage: newtable = copytable(table_to_copy);");
      return NULL;
   }
   if (table->values() == 1)
      copyOfTable = new ConstPField(table->doubleValue());
   else {
      double *values = new double[table->values()];
      table->copyValues(values);
      copyOfTable = new TablePField(values, table->values());
   }
   return _createPFieldHandle(copyOfTable);
}


/* ------------------------------------------------------------- dumptable -- */
double
dumptable(const Arg args[], const int nargs)
{
   int len;
   double *array; 
   FILE  *f = NULL;
   const char  *fname = NULL;

   if (nargs < 1 || nargs > 2)
      return die("dumptable", "Usage: dumptable(table_handle [, out_file])");
   PField *table = (PField *) args[0];
   if (table == NULL)
      return die("dumptable",
                 "First argument must be a handle to the table to dump.");

   if (nargs > 1) {
      if (!args[1].isType(StringType))
         return die("dumptable", "Second argument must be output file name.");
      fname = (const char *) args[1];
      f = fopen(fname, "w+");
      if (f == NULL)
         return die("dumptable",
                    "Can't open file \"%s\": %s.", fname, strerror(errno));
   }
   else
      f = stdout;

   int chars = table->print(f);

   if (f != stdout)
      fclose(f);

   return (chars > 0) ? 0.0 : -1.0;
}

/* ------------------------------------------------------------- plottable -- */
#define DEFAULT_PLOTCMD "with lines"

double
plottable(const Arg args[], const int nargs)
{
#ifdef MACOSX
   static int plot_count = 1;
#endif
   int pause = 10;
   char cmd[256];
   const char *plotcmds = DEFAULT_PLOTCMD;

   if (nargs < 1 || nargs > 3)
      return die("plottable",
                 "Usage: plottable(table_handle [, pause] [, plot_commands])");

   PField *table = (PField *) args[0];
   if (table == NULL)
      return die("plottable", "First argument must be the table to plot.");

   /* 2nd and 3rd args are optional and can be in either order */
   if (nargs > 1) {
      if (args[1].isType(DoubleType))
         pause = (int) args[1];
      else if (args[1].isType(StringType))
         plotcmds = (const char *) args[1];
      else
         return die("plottable",
                    "Second argument can be pause length or plot commands.");
      if (nargs > 2) {
         if (args[2].isType(DoubleType))
            pause = (int) args[2];
         else if (args[2].isType(StringType))
            plotcmds = (const char *) args[2];
         else
            return die("plottable",
                       "Third argument can be pause length or plot commands.");
      }
   }

   char data_file[256] = "/tmp/rtcmix_plot_data_XXXXXX";
   char cmd_file[256] = "/tmp/rtcmix_plot_cmds_XXXXXX";

   if (mkstemp(data_file) == -1 || mkstemp(cmd_file) == -1)
      return die("plottable", "Can't make temp files for gnuplot.");

   FILE *fdata = fopen(data_file, "w");
   FILE *fcmd = fopen(cmd_file, "w");
   if (fdata == NULL || fcmd == NULL)
      return die("dumptable", "Can't open temp files for gnuplot.");

   int chars = table->print(fdata);
   fclose(fdata);
   
   if (chars <= 0)
      return die("dumptable", "Cannot print this kind of table");

   fprintf(fcmd, 
#ifdef MACOSX  /* NB: requires installation of Aquaterm and gnuplot 3.8 */
      "set term aqua %d\n"
#endif
#ifdef TABNAME // FIXME: there *is* no table number, and we don't have var name!
      "set title \"Table %s\"\n"
#else
      "set title \"Table\"\n"
#endif
      "set grid\n"
      "plot '%s' notitle %s\n"
      "!rm '%s' '%s'\n"
      "pause %d\n",
#ifdef MACOSX
// FIXME: ??nevertheless, gnuplots after the first die with bus error
      plot_count++,
#endif
#ifdef TABNAME
      table_name,
#endif
      data_file, plotcmds, data_file, cmd_file, pause);
   fclose(fcmd);

   snprintf(cmd, 255, "gnuplot %s &", cmd_file);
   cmd[255] = 0;
   system(cmd);

   return 0.0;
}

#endif /* PFIELD_CLASS */
