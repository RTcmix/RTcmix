/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifdef PFIELD_CLASS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <rtcmix_types.h>
#include <prototypes.h>
#include <PField.h>
#include <ugens.h>      /* for warn, die */

/* Functions for creating and modifying float arrays.  These can be passed
   from a script to RTcmix functions that can accept them.  All this code
   is derived from RTcmix makegens.

   John Gibson, 1/23/2004
*/

#define MAX_ARRAY_LEN 10000000  /* guard against unintentionally huge allocs */

#ifndef M_PI
  #define M_PI    3.14159265358979323846
#endif
#ifndef TWOPI
  #define TWOPI   M_PI * 2
#endif


typedef enum {
   InvalidTable = -1,
   SndfileTable = 1,
   TextfileTable = 2,
   DatafileTable = 3,
   CurveTable = 4,
   ExpbrkTable = 5,
   LineTable = 6,
   LinebrkTable = 7,
   Wave3Table = 9,
   WaveTable = 10,
   ChebyTable = 17,
   RandomTable = 20,
   WindowTable = 25,
   EndTable
} TableKind;

static char *_table_name[] = {
   NULL,       /* 0 */
   "sndfile",
   "textfile",
   "datafile",
   "curve",    /* 4 */
   "expbrk",
   "line",
   "linebrk",
   NULL,       /* 8 */
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


/* ------------------------------------------------------- local utilities -- */

/* Return 1 if <arg> points to a TablePField; otherwise, return 0. */
static int
_arg_is_table_pfield(const Arg *arg)
{
   if (arg->type != HandleType)
      return 0;
   if (arg->val.handle->type != PFieldType)
      return 0;
   if (dynamic_cast<TablePField *> ((TablePField *) arg->val.handle->ptr) == NULL);
      return 0;
   return 1;
}


/* Given a handle, which we assume points to a valid TablePField, pass back
   the array and length.  Return 0 if no error, -1 if error.
*/
static int
_table_from_handle(Handle *handle, float **array, int *len)
{
   return 0;
}


/* --------------------------------------------------- args_have_same_type -- */
// FIXME: this belongs in a more general file
int
args_have_same_type(const Arg args[], const int nargs, const RTcmixType type)
{
   for (int i = 0; i < nargs; i++)
      if (args[i].type != type)
         return 0;
   return 1;
}


/* ------------------------------------------------------ _normalize_table -- */
/* Rescale the values in <array> so that the maximum absolute value is <peak>.
   Similar to cmix fnscl.
*/
static void
_normalize_table(Float *array, const unsigned int len, const Float peak)
{
   int i;
   double max = 0.0;

   for (i = 0; i < (int) len; i++) {
      double absval = fabs((double) array[i]);
      if (absval > max)
         max = absval;
   }
   max /= peak;
   for (i = 0; i < (int) len; i++)
      array[i] /= (Float) max;
}


/* ----------------------------------------------- _curve_table and helper -- */
/* Derived from gen4 from the UCSD Carl package, described in F.R. Moore, 
   "Elements of Computer Music."  It works like setline, but there's an
   additional argument for each time,value pair (except the last).  This
   arg determines the curvature of the segment, and is called "alpha" in
   the comments to _transition() below.       -JGG, 12/2/01, rev 1/25/04
*/

/* _transition(a, alpha, b, n, output) makes a transition from <a> to <b> in
   <n> steps, according to transition parameter <alpha>.  It stores the
   resulting <n> values starting at location <output>.
      alpha = 0 yields a straight line,
      alpha < 0 yields an exponential transition, and 
      alpha > 0 yields a logarithmic transition.
   All of this in accord with the formula:
      output[i] = a + (b - a) * (1 - exp(i * alpha / (n-1))) / (1 - exp(alpha))
   for 0 <= i < n
*/
static void
_transition(double a, double alpha, double b, int n, Float *output)
{
   int    i;
   double delta, interval = 0.0;

   delta = b - a;

   if (n <= 1) {
      warn("maketable", "'curve' trying to transition over 1 array slot; "
                                       "time between points is too short");
      *output = (Float) a;
      return;
   }
   interval = 1.0 / (n - 1.0);

   if (alpha != 0.0) {
      double denom = 1.0 / (1.0 - exp((double) alpha));
      for (i = 0; i < n; i++)
         *output++ = (Float) (a + delta
                        * (1.0 - exp((double) i * alpha * interval)) * denom);
   }
   else
      for (i = 0; i < n; i++)
         *output++ = (Float) (a + delta * i * interval);
}

#define MAX_CURVE_PTS 256

static int
_curve_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   int    i, points, seglen = 0;
   double factor;
   Float  *ptr;
   double time[MAX_CURVE_PTS], value[MAX_CURVE_PTS], alpha[MAX_CURVE_PTS];

   if (nargs < 5 || (nargs % 3) != 2) {      /* check number of args */
      die("maketable", "'curve' usage: t1 v1 a1 ... tn vn");
      return -1;
   }
   if ((nargs / 3) + 1 > MAX_CURVE_PTS) {
      die("maketable", "too many arguments for curve.");
      return -1;
   }
   if (!args_have_same_type(args, nargs, FloatType)) {
      die("maketable", "<time, value, alpha> pairs must be numbers.");
      return -1;
   }
   if (args[0].val.number != 0.0) {
      die("maketable", "'curve' first time must be zero.");
      return -1;
   }

   for (i = points = 0; i < nargs; points++) {
      time[points] = args[i++].val.number;
      if (points > 0 && time[points] < time[points - 1])
         goto time_err;
      value[points] = args[i++].val.number;
      if (i < nargs)
         alpha[points] = args[i++].val.number;
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
   die("maketable", "times not in ascending order.");
   return -1;
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
_line_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   double scaler, starttime, thistime, thisval, nexttime, nextval, endtime;
   int i, j, k, l;

   if (!args_have_same_type(args, nargs, FloatType)) {
      die("maketable", "<time, value> pairs must be numbers.");
      return -1;
   }
   if ((nargs % 2) != 0) {
      die("maketable", "incomplete <time, value> pair.");
      return -1;
   }

   endtime = args[nargs - 2].val.number;
   starttime = args[0].val.number;
   if (endtime - starttime <= 0.0)
      goto time_err;
#ifdef NEWWAY
   scaler = (double) (len - 1) / (endtime - starttime);
#else
   scaler = (double) len / (endtime - starttime);
#endif
   nextval = args[1].val.number;
   thistime = starttime;
   i = 0;
   for (k = 1; k < nargs; k += 2) {
      thisval = nextval;
      if (k < nargs - 1) {
         nexttime = args[k + 1].val.number - starttime;
         if (nexttime - thistime < 0.0)   /* okay for them to be the same */
            goto time_err;
         nextval = args[k + 2].val.number;
      }
      else
         nextval = nexttime = 0.0;     /* don't read off end of args array */
      j = i + 1;
#ifdef NEWWAY
      i = (int) (nexttime * scaler);
#else
      i = (int) ((nexttime * scaler) + 1.0);
#endif
      for (l = j; l <= i; l++) {
         if (l <= (int) len)
            array[l - 1] = (Float) (thisval + (nextval - thisval)
                                          * (double) (l - j) / ((i - j) + 1));
      }
   }
#ifdef NEWWAY
   array[len - 1] = args[nargs - 1].val.number;
#endif

   return 0;
time_err:
   die("maketable", "times not in ascending order.");
   return -1;
}


/* ---------------------------------------------------------- _wave3_table -- */
/* Similar to cmix gen 9, but no normalization.
*/
static int
_wave3_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   int i, j;

   for (i = 0; i < (int) len; i++)
      array[i] = 0.0;

   for (j = nargs - 1; j > 0; j -= 3) {
      assert(j > 0);
      assert(j < nargs);
      if (args[j - 1].val.number != 0.0) {
         for (i = 0; i < (int) len; i++) {
            double val = sin(TWOPI * ((double) i
                                 / ((double) len / args[j - 2].val.number)
                                 + args[j].val.number / 360.0));
            array[i] += (Float) (val * args[j - 1].val.number);
         }
      }
   }

   return 0;
}


/* ----------------------------------------------------------- _wave_table -- */
/* Equivalent to cmix gen 10.
*/
static int
_wave_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   unsigned int i, j;

   for (i = 0; i < len; i++)
      array[i] = 0.0;
   j = nargs;
   while (j--) {
      if (args[j].type != FloatType) {
         die("maketable", "Harmonic amplitudes must be numbers.");
         return -1;
      }
      if (args[j].val.number != 0.0) {
         for (i = 0; i < len; i++) {
            double val = TWOPI * (double) i / (double) len / (double) (j + 1);
            array[i] += (Float) (sin(val) * args[j].val.number);
         }
      }
   }
// FIXME: ?? should be optional in maketable, for consistency...
// _normalize_table(array, len, 1.0);

   return 0;
}


/* ---------------------------------------------------------- _cheby_table -- */
/* Equivalent to cmix gen 17.  Computes transfer function using Chebyshev
   polynomials.  First argument is the index value for which the function will
   create the harmonics specified by the following arguments.
*/
static int
_cheby_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   int i, j;
   double Tn, Tn1, Tn2, x, d;

   d = (double) ((len / 2) - 0.5);
   for (i = 0; i < (int) len; i++) {
      x = (i / d - 1.0) / args[0].val.number;
      array[i] = 0.0;
      Tn1 = 1.0;
      Tn = x;
      for (j = 1; j < nargs; j++) {
         array[i] += args[j].val.number * Tn;
         Tn2 = Tn1;
         Tn1 = Tn;
         Tn = (2.0 * x * Tn1) - Tn2;
      }
   }

   return 0;
}


/* --------------------------------------------------------- _window_table -- */
/* Equivalent to cmix gen 25.
*/
static int
_window_table(const Arg args[], const int nargs, Float *array,
   const unsigned int len)
{
   unsigned int i, window_type = 0;

   if (nargs != 1) {
      die("maketable", "Missing window type.");
      return -1;
   }
   if (args[0].type == StringType) {
      if (strcmp(args[0].val.string, "hanning") == 0)
         window_type = 1;
      else if (strcmp(args[0].val.string, "hamming") == 0)
         window_type = 2;
      else {
         die("maketable", "Unsupported window type '%s'.", args[0].val.string);
         return -1;
      }
   }
   else if (args[0].type != FloatType) {
      die("maketable", "Window type must be a string or numeric code.");
      return -1;
   }

   switch (window_type) {
      case 1:     /* hanning window */
         for (i = 0; i < len; i++)
            array[i] = -cos(2.0 * M_PI * (double) i / (double) len) * 0.5 + 0.5;
         break;
      case 2:     /* hamming window */
         for (i = 0; i < len; i++) {
            double val = cos(2.0 * M_PI * (double) i / (double) len);
            array[i] = 0.54 - 0.46 * val;
         }
         break;
      default:
         die("maketable", "Unsupported window type (%d).", window_type);
         return -1;
   }
// FIXME: should be optional in maketable, for consistency...
// _normalize_table(array, len, 1.0);

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
   Float *array, unsigned int *len)
{
   int status;
   TableKind tablekind;

   /* Call the appropriate factory function, skipping over first two args. */
   if (args[0].type == FloatType)
      tablekind = (TableKind) args[0].val.number;
   else if (args[0].type == StringType) {
      tablekind = _string_to_tablekind(args[0].val.string);
      if (tablekind == InvalidTable) {
         die("maketable", "Invalid table type string '%s'", args[0].val.string);
         return -1;
      }
   }
   else {
      die("maketable", "First argument must be a number or string.");
      return -1;
   }

   switch (tablekind) {
      case SndfileTable:
         goto unimplemented;
         break;
      case TextfileTable:
         goto unimplemented;
         break;
      case DatafileTable:
         goto unimplemented;
         break;
      case CurveTable:
         status = _curve_table(&args[startarg], nargs - startarg, array, *len);
         break;
      case ExpbrkTable:
         goto unimplemented;
         break;
      case LineTable:
         status = _line_table(&args[startarg], nargs - startarg, array, *len);
         break;
      case LinebrkTable:
         goto unimplemented;
         break;
      case Wave3Table:
         status = _wave3_table(&args[startarg], nargs - startarg, array, *len);
         break;
      case WaveTable:
         status = _wave_table(&args[startarg], nargs - startarg, array, *len);
         break;
      case ChebyTable:
         status = _cheby_table(&args[startarg], nargs - startarg, array, *len);
         break;
      case RandomTable:
         goto unimplemented;
         break;
      case WindowTable:
         status = _window_table(&args[startarg], nargs - startarg, array, *len);
         break;
      default:
         die("maketable", "invalid table type.");
         return -1;
         break;
   }
   return status;
unimplemented:
   die("maketable", "unimplemented table type.");
   return -1;
}

extern "C" {
	Handle maketable(const Arg args[], const int nargs);
	Float normtable(const Arg args[], const int nargs);
	Float plottable(const Arg args[], const int nargs);
	Float dumptable(const Arg args[], const int nargs);
};

/* ------------------------------------------------------------- maketable -- */
Handle
maketable(const Arg args[], const int nargs)
{
   int status, lenindex, normalize = 0;
   unsigned int len;
   Float *data;
   Handle handle;

   if (nargs < 2) {
      die("maketable", "Requires at least two arguments.");
      return NULL;
   }
   if (args[1].type == StringType) {
      if (strncmp(args[1].val.string, "norm", 4) == 0)
         normalize = 1;
      else {
         die("maketable", "Invalid string option.");
         return NULL;
      }
      lenindex = 2;
   }
   else
      lenindex = 1;
   if (args[lenindex].type != FloatType) {
      die("maketable", "%s argument must be length of table.",
         lenindex == 1 ? "Second" : "Third");
      return NULL;
   }
   len = (unsigned int) args[lenindex].val.number;
   if (len > MAX_ARRAY_LEN) {
      warn("maketable", "Requesting larger than maximum table length.  "
                                          "Setting to %d.", MAX_ARRAY_LEN);
      len = MAX_ARRAY_LEN;
   }

   // Allocate table array.  TablePField will own and delete this.
   
   data = new Float[len];
   if (data == NULL) {
      die("maketable", "Out of memory.");
      return NULL;
   }

   status = _dispatch_table(args, nargs, lenindex + 1, data, &len);

   if (normalize)
      _normalize_table(data, len, 1.0);

   handle = (Handle) malloc(sizeof(struct _handle));
   handle->type = PFieldType;
   handle->ptr = (void *) new TablePField(data, len);

   return handle;
}


/* ------------------------------------------------------------- normtable -- */
//FIXME: should return type be Handle? See below.
Float
normtable(const Arg args[], const int nargs)
{
   int copy_table = 0;
   Float peak = 1.0;

   if (nargs < 1) {
      die("normtable", "Requires at least one argument (table to normalize).");
      return -1.0;
   }
   if (!_arg_is_table_pfield(&args[0])) {
      die("normtable", "First argument must be the table to normalize.");
      return -1.0;
   }
   if (nargs > 1 && args[1].type == FloatType)
      peak = args[1].val.number;
   if (nargs > 2 && args[2].type == FloatType)
      copy_table = (args[2].val.number == 1.0);
#ifdef NOTYET
// FIXME: this would require returning a Handle instead of a Float
   if (copy_table) {
   }
#endif
   _normalize_table(args[0].val.array->data, args[0].val.array->len, peak);
   return 0.0;
}


/* ------------------------------------------------------------- dumptable -- */
Float
dumptable(const Arg args[], const int nargs)
{
   int   len, status;
   float *array; 
   FILE  *f = NULL;
   char  *fname = NULL;

   if (nargs < 1 || nargs > 2) {
      die("dumptable", "Usage: dumptable(table_handle [, out_file])");
      return -1.0;
   }
   if (!_arg_is_table_pfield(&args[0])) {
      die("dumptable", "First argument must be a handle to the table to dump.");
      return -1.0;
   }

   if (nargs > 1) {
      if (args[1].type != StringType) {
         die("dumptable", "Second argument must be output file name.");
         return -1.0;
      }
      fname = args[1].val.string;
      f = fopen(fname, "w+");
      if (f == NULL) {
         perror("dumptable");
         return -1.0;
      }
   }
   else
      f = stdout;

#ifdef NOTYET
   status = _table_from_handle(args[0].val.handle, &array, &len);
   if (status == 0) {
      int i;
      // FIXME: interesting: no way to identify this better for user!
      printf("Dumping function table ...\n");
      for (i = 0; i < len; i++)
         fprintf(f, "%d %.6f\n", i, array[i]);
   }
   else
      die(NULL, "Unknown problem dumping table.");
#endif

   if (f != stdout)
      fclose(f);

   return 0.0;
}


/* ------------------------------------------------------------- plottable -- */
Float
plottable(const Arg args[], const int nargs)
{
   if (nargs != 1) {
      die("plottable", "Requires one argument (table to plot).");
      return -1.0;
   }
   if (!_arg_is_table_pfield(&args[0])) {
      die("plottable", "First argument must be the table to plot.");
      return -1.0;
   }
   return 0.0;
}

#endif /* PFIELD_CLASS */
