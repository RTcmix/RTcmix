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
   return 1;
}

static PField *
_getPField(const Arg *arg)
{
	PField *pf = NULL;
	if (_arg_is_table_pfield(arg)) {
		pf = (PField *) (arg->val.handle->ptr);
	}
	return pf;
}

static TablePField *
_getTablePField(const Arg *arg)
{
	PField *tpf = NULL;
	if ((tpf = _getPField(arg)) != NULL) {
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
      if (args[i].type != type)
         return 0;
   return 1;
}


/* ------------------------------------------------------ _normalize_table -- */
/* Rescale the values in <array> so that the maximum absolute value is <peak>.
   Similar to cmix fnscl.
*/
static void
_normalize_table(double *array, const unsigned int len, const double peak)
{
   unsigned int i;
   double max = 0.0;

   for (i = 0; i < len; i++) {
      double absval = fabs(array[i]);
      if (absval > max)
         max = absval;
   }
   max /= peak;
   for (i = 0; i < len; i++)
      array[i] /= max;
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
_transition(double a, double alpha, double b, int n, double *output)
{
   int    i;
   double delta, interval = 0.0;

   delta = b - a;

   if (n <= 1) {
      warn("maketable", "'curve' trying to transition over 1 array slot; "
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
_curve_table(const Arg args[], const int nargs, double *array,
   const unsigned int len)
{
   int    i, points, seglen = 0;
   double factor, *ptr;
   double time[MAX_CURVE_PTS], value[MAX_CURVE_PTS], alpha[MAX_CURVE_PTS];

   if (nargs < 5 || (nargs % 3) != 2) {      /* check number of args */
      die("maketable", "'curve' usage: t1 v1 a1 ... tn vn");
      return -1;
   }
   if ((nargs / 3) + 1 > MAX_CURVE_PTS) {
      die("maketable", "too many arguments for curve.");
      return -1;
   }
   if (!args_have_same_type(args, nargs, DoubleType)) {
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
_line_table(const Arg args[], const int nargs, double *array,
   const unsigned int len)
{
   double scaler, starttime, thistime, thisval, nexttime, nextval, endtime;
   int i, j, k, l;

   if (!args_have_same_type(args, nargs, DoubleType)) {
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
            array[l - 1] = thisval + (nextval - thisval)
                                          * (double) (l - j) / ((i - j) + 1);
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
_wave3_table(const Arg args[], const int nargs, double *array,
   const unsigned int len)
{
   unsigned int i;
   int j;

   for (i = 0; i < len; i++)
      array[i] = 0.0;

   for (j = nargs - 1; j > 0; j -= 3) {
      assert(j > 0);
      assert(j < nargs);
      if (args[j - 1].val.number != 0.0) {
         for (i = 0; i < len; i++) {
            double val = sin(TWOPI * ((double) i
                                 / ((double) len / args[j - 2].val.number)
                                 + args[j].val.number / 360.0));
            array[i] += (val * args[j - 1].val.number);
         }
      }
   }

   return 0;
}


/* ----------------------------------------------------------- _wave_table -- */
/* Equivalent to cmix gen 10.
*/
static int
_wave_table(const Arg args[], const int nargs, double *array,
   const unsigned int len)
{
   unsigned int i, j;

   for (i = 0; i < len; i++)
      array[i] = 0.0;
   j = nargs;
   while (j--) {
      if (args[j].type != DoubleType) {
         die("maketable", "Harmonic amplitudes must be numbers.");
         return -1;
      }
      if (args[j].val.number != 0.0) {
         for (i = 0; i < len; i++) {
            double val = TWOPI * (double) i / (len / (j + 1));
            array[i] += (sin(val) * args[j].val.number);
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
_cheby_table(const Arg args[], const int nargs, double *array,
   const unsigned int len)
{
   unsigned int i;
   int j;
   double Tn, Tn1, Tn2, x, d;

   d = (double) ((len / 2) - 0.5);
   for (i = 0; i < len; i++) {
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
_window_table(const Arg args[], const int nargs, double *array,
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
   else if (args[0].type != DoubleType) {
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
   double *array, unsigned int *len)
{
   int status;
   TableKind tablekind;

   /* Call the appropriate factory function, skipping over first two args. */
   if (args[0].type == DoubleType)
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
	Handle normtable(const Arg args[], const int nargs);
	Handle multtable(const Arg args[], const int nargs);
	Handle addtable(const Arg args[], const int nargs);
	double plottable(const Arg args[], const int nargs);
	double dumptable(const Arg args[], const int nargs);
};

/* ------------------------------------------------------------- maketable -- */
Handle
maketable(const Arg args[], const int nargs)
{
   int status, lenindex, normalize = 0;
   unsigned int len;
   double *data;
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
   if (args[lenindex].type != DoubleType) {
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
   
   data = new double[len];
   if (data == NULL) {
      die("maketable", "Out of memory.");
      return NULL;
   }

   status = _dispatch_table(args, nargs, lenindex + 1, data, &len);

   if (normalize)
      _normalize_table(data, len, 1.0);

   return _createPFieldHandle(new TablePField(data, len));
}

/* ------------------------------------------------------------- multtable -- */
Handle
multtable(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *table0 = _getPField(&args[0]);
		PField *table1 = _getPField(&args[1]);
		if (!table1) {
			if (args[1].type == DoubleType) {
				table1 = new ConstPField(args[1].val.number);
			}
		}
		else if (!table0) {
			if (args[0].type == DoubleType) {
				table0 = new ConstPField(args[0].val.number);
			}
		}
		if (table0 && table1) {
			return _createPFieldHandle(new MultPField(table0, table1));
		}
	}
	die("multtable", "Usage: mul(table1, table2) or mul(table1, const1)");
	return NULL;
}

/* ------------------------------------------------------------- multtable -- */
Handle
addtable(const Arg args[], const int nargs)
{
	if (nargs == 2) {
		PField *table0 = _getPField(&args[0]);
		PField *table1 = _getPField(&args[1]);
		if (!table1) {
			if (args[1].type == DoubleType) {
				table1 = new ConstPField(args[1].val.number);
			}
		}
		else if (!table0) {
			if (args[0].type == DoubleType) {
				table0 = new ConstPField(args[0].val.number);
			}
		}
		if (table0 && table1) {
			return _createPFieldHandle(new AddPField(table0, table1));
		}
	}
	die("multtable", "Usage: mul(table1, table2) or mul(table1, const1)");
	return NULL;
}


/* ------------------------------------------------------------- normtable -- */
Handle
normtable(const Arg args[], const int nargs)
{
   int copy_table = 0;
   double peak = 1.0;

   if (nargs < 1) {
      die("normtable", "Requires at least one argument (table to normalize).");
      return NULL;
   }
   TablePField *table = _getTablePField(&args[0]);
   if (table == NULL) {
      die("normtable", "First argument must be a handle to the table to normalize.");
      return NULL;
   }
   if (nargs > 1 && args[1].type == DoubleType)
      peak = args[1].val.number;
   if (nargs > 2 && args[2].type == DoubleType)
      copy_table = (args[2].val.number == 1.0);
	TablePField *tableToNormalize = NULL;
#ifdef NOTYET
	if (copy_table) {
   		tableToNormalize = table->copy();
	}
	else {
		tableToNormalize = table;
	}
	tableToNormalize->normalize(peak);
#endif
//   _normalize_table(args[0].val.array->data, args[0].val.array->len, peak);
	return _createPFieldHandle(tableToNormalize);
}


/* ------------------------------------------------------------- dumptable -- */
double
dumptable(const Arg args[], const int nargs)
{
   int len;
   double *array; 
   FILE  *f = NULL;
   const char  *fname = NULL;

   if (nargs < 1 || nargs > 2) {
      die("dumptable", "Usage: dumptable(table_handle [, out_file])");
      return -1.0;
   }
   PField *table = _getPField(&args[0]);
   if (table == NULL) {
      die("dumptable", "First argument must be a handle to the table to dump.");
      return -1.0;
   }

   if (nargs > 1) {
      if (args[1].type != StringType) {
         die("dumptable", "Second argument must be output file name.");
         return -1.0;
      }
      fname = (const char *) args[1];
      f = fopen(fname, "w+");
      if (f == NULL) {
         perror("dumptable");
         return -1.0;
      }
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

   if (nargs < 1 || nargs > 3) {
      die("plottable",
         "Usage: plottable(table_handle [, pause] [, plot_commands])");
      return -1.0;
   }
   PField *table = _getPField(&args[0]);
   if (table == NULL) {
      die("plottable", "First argument must be the table to plot.");
      return -1.0;
   }

   /* 2nd and 3rd args are optional and can be in either order */
   if (nargs > 1) {
      if (args[1].type == DoubleType)
         pause = (int) args[1];
      else if (args[1].type == StringType)
         plotcmds = (const char *) args[1];
      else {
         die("plottable",
            "Second argument can be pause length or plot commands.");
         return -1.0;
      }
      if (nargs > 2) {
         if (args[2].type == DoubleType)
            pause = (int) args[2].val.number;
         else if (args[2].type == StringType)
            plotcmds = args[2].val.string;
         else {
            die("plottable",
               "Third argument can be pause length or plot commands.");
            return -1.0;
         }
      }
   }

   char data_file[256] = "/tmp/rtcmix_plot_data_XXXXXX";
   char cmd_file[256] = "/tmp/rtcmix_plot_cmds_XXXXXX";

   if (mkstemp(data_file) == -1 || mkstemp(cmd_file) == -1) {
      die("plottable", "Can't make temp files for gnuplot.");
      return -1.0;
   }
   FILE *fdata = fopen(data_file, "w");
   FILE *fcmd = fopen(cmd_file, "w");
   if (fdata == NULL || fcmd == NULL) {
      die("dumptable", "Can't open temp files for gnuplot.");
      return -1.0;
   }

   int chars = table->print(fdata);
   fclose(fdata);
   
	if (chars <= 0) {
		die("dumptable", "Cannot print this kind of table");
		return -1;
	}

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
