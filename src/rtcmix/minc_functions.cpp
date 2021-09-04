#include "globals.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <ugens.h>
#include <spray.h>
#include <sfheader.h>
#include <maxdispargs.h>
#include <Option.h>
#include "prototypes.h"

#define ARRAY_SIZE 256
#define NUM_ARRAYS  32
static double minc_array[NUM_ARRAYS][ARRAY_SIZE],minc_array_size[NUM_ARRAYS];

extern "C" {

double m_boost(float p[])
{ return(boost(p[0])); }

double m_midipch(float p[])
{ return(midipch(p[0])); }

double m_pchmidi(float p[])
{ return(pchmidi(p[0])); }

double m_midioct(float p[])
{ return(midioct(p[0])); }

double m_octmidi(float p[])
{ return(octmidi(p[0])); }

double m_midicps(float p[])
{ return(midicps(p[0])); }

double m_cpsmidi(float p[])
{ return(cpsmidi(p[0])); }

double m_cpspch(float p[])
{ return(cpspch(p[0])); }

double m_pchoct(float p[])
{ return(pchoct(p[0])); }

double m_octpch(float p[])
{ return(octpch(p[0])); }

double m_pchcps(float p[])
{ return(pchcps(p[0])); }

double m_cpsoct(float p[])
{ return(cpsoct(p[0])); }

double m_octcps(float p[])
{ return(octcps(p[0])); }

double m_octlet(float p[], int nargs, double pp[])
{
	if (nargs > 0 && pp[0] > 0.0)
		return octlet((unsigned char *) DOUBLE_TO_STRING(pp[0]));
	die("octlet", "usage: octlet(\"pitch\"), where pitch is \"Ab3\", etc.");
    RTExit(PARAM_ERROR);
	return 8.00;
}

double m_cpslet(float p[], int nargs, double pp[])
{
	if (nargs > 0 && pp[0] > 0.0)
		return cpslet((unsigned char *) DOUBLE_TO_STRING(pp[0]));
	die("cpslet", "usage: cpslet(\"pitch\"), where pitch is \"Ab3\", etc.");
    RTExit(PARAM_ERROR);
	return 0.0;
}

double m_pchlet(float p[], int nargs, double pp[])
{
	if (nargs > 0 && pp[0] > 0.0)
		return pchlet((unsigned char *) DOUBLE_TO_STRING(pp[0]));
	die("pchlet", "usage: pchlet(\"pitch\"), where pitch is \"Ab3\", etc.");
    RTExit(PARAM_ERROR);
	return 8.00;
}

double m_pchadd(float p[], int nargs, double pp[])
{
	return pchoct(octpch(pp[0]) + octpch(pp[1]));
}

double m_rand()
{ return rrand(); }

double m_random()
{ return (rrand() * 0.5) + 0.5; }

double m_srand(float p[], int n_args)
{
   unsigned int randx;

   if (n_args == 0) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      randx = (unsigned int) tv.tv_usec;
	  rtcmix_advise("srand", "Seed initialized internally with value %u", randx);
   }
   else
      randx = (unsigned int) p[0];
   srrand(randx);
   return 0.0;
}

double m_time_beat(float p[])
{ float time_beat(float); return(time_beat(p[0])); }

double m_beat_time(float p[])
{ float beat_time(float); return(beat_time(p[0])); }

double m_trunc(float p[], int nargs, double pp[])
{ return((double)(long long)(pp[0])); }		// D.S. 08/09

double m_ampdb(float p[])
{ return(ampdb(p[0])); }

double m_dbamp(float p[])
{ return(dbamp(p[0])); }

double m_stringify(float p, int n_args, double pp[])
{
	/* coerces a string passed in from Minc in quotes in p[0]
	   to a 'floating point' pointer suitable for use in
	   further cmix calls */
    if (n_args < 1) {
        die("stringify", "usage: stringfy(\"some_quoted_string\")");
        RTExit(PARAM_ERROR);
    }
    return (pp[0] == 0.0) ? 0 : STRINGIFY(pp[0]);
}

double m_log(float p[], int n_args)
{
   double val;
    if (p[0] <= 0.0) {
        die("log", "argument cannot be <= 0");
        RTExit(PARAM_ERROR);
    }
    val = log10((double)p[0]);

   return(val);
}

double m_ln(float p[], int n_args)
{
   double val;

    if (p[0] <= 0.0) {
        die("ln", "argument cannot be <= 0");
        RTExit(PARAM_ERROR);
    }
   val = log((double)p[0]);

   return(val);
}

double m_pow(float p[], int n_args, double pp[])
{
	double val;

	val = pow(pp[0], pp[1]);
	return(val);
}

double m_sqrt(float p[], int n_args, double pp[])
{
	double val;

    if (p[0] <= 0.0) {
        die("sqrt", "argument cannot be < 0");
        RTExit(PARAM_ERROR);
    }
	val = sqrt(pp[0]);
	return(val);
}

double m_round(float p[], int n_args)
{
	int val = p[0] + .5;
	return (double) val;
}

double m_wrap(float p[], int n_args)
{
	/* keeps value between 0 and p[1] */
	int val = p[0];
	int range = (int) p[1];
	if(p[1] >= 1.0) {
		while(val > range) val -= range;
		while(val < 0) val += range;
	}
	else if(p[1] <= -1.0) {
		while(val < range) val -= range;
		while(val > 0) val += range;
	}
	else val = 0;
	return (double) val;
}

double m_print(float p[], int n_args, double pp[])
{
// BGG -- NOTE:  This is now done in parser/minc/builtin.c
	RTPrintf("Value = %10.8f\n", pp[0]);
	return 0.0;
}

double m_abs(float p[], int n_args)
{
	return((p[0] >= 0.0) ? p[0] : -(p[0]));
}

double m_mod(float p[], int n_args)
{
	int i;
    if (p[1] < 1.0) {
        die("mod", "Modulus must be >= 1");
        RTExit(PARAM_ERROR);
    }
	i = (int)p[0] % (int)p[1];
	return((float)i);
}

double m_max(float p[], int n_args)
{
	int i;
	float max = -1e+22;
	for(i=0; i<n_args; i++)
		if(p[i] > max) max=p[i];
	return(max);
}

double m_min(float p[], int n_args)
{
	int i;
	float min = 1e+22;
	for(i=0; i<n_args; i++)
		if(p[i] < min) min=p[i];
	return(min);
}

double m_exit(float p[], int n_args, double pp[])
{
	const char *message = DOUBLE_TO_STRING(p[0]);
	if (message) {
		rtcmix_warn(NULL, "%s -- exiting if allowed", message);
	}
	closesf();
	return p[1];
}

double m_load_array(float p[], int n_args, double pp[])
{
	int i,j;
	if(n_args > ARRAY_SIZE) n_args = ARRAY_SIZE+1;
	j = p[0];
	for(i=1; i<n_args; i++) minc_array[j][i-1] = pp[i];
	minc_array_size[j] = n_args;
	return(n_args-1);
}

double m_get_array(float p[], int n_args)
{
	int i, size, index;

	i = p[0];
	size = minc_array_size[i];
	index = (int) p[1];

	if (index >= size)
		index = size - 1;

	return (minc_array[i][index]);
}

double m_put_array(float p[], int n_args)
{ /* to load a single value from minc */
	int i,j;
	i=p[0];
	j=p[1];
	minc_array[i][j] = p[2];
	minc_array_size[i]=minc_array_size[i] > j+1 ? minc_array_size[i] : j+1;
	if(j < ARRAY_SIZE) return(j);
	else return(-1);
}

double m_get_sum(float p[], int n_args)
{
	int i,j,k;
	float sum;
	i = p[0];
	if((j=p[1]) >= minc_array_size[i]) return(0);
	for(k=0,sum=0; k<=j; k++) sum += minc_array[i][k];
	return(sum);
}

double m_get_size(float p[], int n_args)
{
	/* returns same value as load_array would */
	return((double)minc_array_size[(int)p[0]]-1);
}

double m_getpch(float p[], int n_args, double pp[])
{
	int pchfd;
	int frameno,nbframe;
	long skipbytes;
	float vals[200]; /* enough for 46 poles + 4 data values */
	char  *input;

/*	p0=name of pchanal file; p1=framenumber to get */

	input = DOUBLE_TO_STRING(pp[0]);

    if((pchfd = open(input,0)) < 0) {
		die("getpch", "Can't open pitch analysis file");
        RTExit(FILE_ERROR);
    }

	nbframe = 2*FLOAT; 
	frameno = (int)p[1];

	skipbytes = frameno * nbframe;
    if (lseek(pchfd, skipbytes, 0) < 0) {
		die("getpch", "Error on pchanal lseek");
        RTExit(FILE_ERROR);
    }

    if (read(pchfd, (char *)vals, nbframe) != nbframe) {
		die("getpch", "Bad read on pchanal file");
        RTExit(FILE_ERROR);
    }

	close(pchfd);

	return((double) vals[0]);
}

double m_getamp(float p[], int n_args, double pp[])
{
	int pchfd;
	int frameno,nbframe;
	long skipbytes;
	float vals[200]; /* enough for 46 poles + 4 data values */
	char  *input;

/*	p0=name of pchanal file; p1=framenumber to get */

	input = DOUBLE_TO_STRING(pp[0]);

    if((pchfd = open(input,0)) < 0) {
		die("getamp", "Can't open pitch analysis file");
        RTExit(FILE_ERROR);
    }

	nbframe = 2*FLOAT; 
	frameno = (int)p[1];

	skipbytes = frameno * nbframe;
    if (lseek(pchfd, skipbytes, 0) < 0) {
        close(pchfd);
		die("getamp", "Error on pchanal lseek");
        RTExit(FILE_ERROR);
    }

    if (read(pchfd, (char *)vals, nbframe) != nbframe) {
        close(pchfd);
		die("getamp", "Bad read on pchanal file");
        RTExit(FILE_ERROR);
    }

	close(pchfd);

	return((double) vals[1]);
}

double m_print_is_on(float p[], int n_args)
{
	if (n_args > 0)
		set_double_option(kOptionPrint, p[0]);
	else
		set_double_option(kOptionPrint, MMP_PRINTALL);
	return 1.0;
}

double m_print_is_off(float p[], int n_args)
{
	set_double_option(kOptionPrint, 0.0);
	return 0.0;
}

static struct slist slist[NUM_SPRAY_ARRAYS];

double m_get_spray(float p[], int n_args)
{
	int table_num = (int) p[0];

    if (table_num < 0 || table_num >= NUM_SPRAY_ARRAYS) {
        die("get_spray", "Spray table number must be between 0 and %d.",
                                                   NUM_SPRAY_ARRAYS - 1);
        RTExit(PARAM_ERROR);
    }
    if (slist[table_num].size == 0) {
        die("get_spray", "Spray table number %d was not initialized.", table_num);
        RTExit(PARAM_ERROR);
    }

	return (double) (spray(&slist[table_num]));
}

double m_spray_init(float p[], int n_args)
{
	int   table_num, size;
   unsigned int seed;

	table_num = (int) p[0];
    if (table_num < 0 || table_num >= NUM_SPRAY_ARRAYS) {
        die("spray_init", "Spray table number must be between 0 and %d.",
                                                   NUM_SPRAY_ARRAYS - 1);
        RTExit(PARAM_ERROR);
    }
   size = (int) p[1];
    if (size < 2 || size > MAX_SPRAY_SIZE) {
        die("spray_init", "Spray table size must be between 2 and %d.",
                                                   MAX_SPRAY_SIZE);
        RTExit(PARAM_ERROR);
    }
   seed = (unsigned int) p[2];

	sprayinit(&slist[table_num], size, seed);
	return 0.0;
}


static int line_array_size = 1000;      /* modified by m_setline_size */

double m_setline_size(float p[], int n_args)
{
    if (p[0] < 2) {
		die("setline_size", "Setline array size must be at least 2!");
        RTExit(PARAM_ERROR);
    }
	line_array_size = p[0];
	rtcmix_advise("setline_size", "Setline arrays will have %d elements.",
                                                      line_array_size);
	return 0.0;
}


double m_setline(float p[], int n_args)
{
	float	pp[MAXDISPARGS];
	int	i;

	pp[0] = 1;
	pp[1] = 18;           /* not sure whether this should be gen18 or gen24 */
	pp[2] = line_array_size;

	for (i = 0; i < n_args; i++)
		pp[i+3] = p[i];

	makegen(pp, n_args+3, NULL);

	return 0.0;
}

/* create exponential curve */

double m_setexp(float p[], int n_args)
{
	float	pp[MAXDISPARGS];
	float   prevloc, minloc, locRange;
	int	i;

	pp[0] = 1;
	pp[1] = 5;           /* gen 5 creates exponential curve */
	pp[2] = line_array_size;

    minloc = p[0];	/* loc from first <loc, value> pair */
	prevloc = minloc;
    locRange = p[n_args - 2] - minloc;	/* delta betw. first & last loc */

	/* copy args, but guard against negatives and zeroes! */
	/* we convert <loc, val>, <loc2, val2>, ... into
	 * val, deltaloc1, val2, deltaloc2, ... 
	 */
	for (i = 1; i < n_args - 1; i += 2)
	{
		float val = p[i];
		float loc = p[i+1];
		if (loc < prevloc) {
			die("setexp", "Invalid time arguments");
            RTExit(PARAM_ERROR);
		}
		pp[i+2] = val > 0.0f ? val : 0.00001;
		pp[i+3] = (int) (line_array_size * ((loc - prevloc) / locRange));
		prevloc = loc;
	}
	/* add final value to arg list */
	pp[i+2] = p[i] > 0.0f ? p[i] : 0.00001;

	makegen(pp, n_args+2, NULL);

	return 0.0;
}

/* Note: MIX used 200 as default; WAVETABLE used 1000. It needs a faster
   rate, since it's changing an oscillator at that speed, rather than just
   an amplitude envelope. I'm just making 1000 the default for everyone.  -JGG
*/
int resetval = 1000;                 /* modified by m_reset; read by insts */

double m_reset(float p[], int n_args)
{
    if (p[0] <= 0) {
		die("reset", "Control rate must be greater than 0!");
        RTExit(PARAM_ERROR);
    }
	resetval = p[0];
	rtcmix_advise("reset", "Control rate set to %d updates per second.", resetval);

	return 0.0;
}

/* returns a randomly-interpolated value between its two input values, inclusive */
static double _irand(double min, double max)
{
	double frac = m_random();
	return (frac * min) + (1.0 - frac) * max;
}

/* Return a random float between min and max, inclusive. If max > min,
   the two are exchanged. If only one arg is present, it is max, and
   min is set to zero.
*/
double m_irand(float p[], int n_args, double pp[])
{
	double min, max;
	if (n_args == 1) {
		min = 0.0;
		max = pp[0];
	}
	else if (n_args == 2) {
		min = pp[0];
		max = pp[1];
	}
	else {
		die("irand", "Usage: irand([min,] max)\nDefault <min> is zero\n");
        RTExit(PARAM_ERROR);
	}
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	return _irand(min, max);
}

/* Assuming min < max, return a random integer between min and max.
   The range is inclusive of min if min >= 0. The range is inclusive
   of max if max <= 0. If min > max, the two are exchanged. If only
   one arg is present, it is max, and min is set to zero. Examples,
		min = 0, max = 10  =>  return integer between 0 and 9 (inclusive)
		min = -10, max = 0  =>  return integer between -9 and 0 (inclusive)
		min = -10, max = 10  =>  return integer between -9 and 9 (inclusive)
*/
double m_trand(float p[], int n_args, double pp[])
{
	double min, max;
	if (n_args == 1) {
		min = 0.0;
		max = pp[0];
	}
	else if (n_args == 2) {
		min = pp[0];
		max = pp[1];
	}
	else {
		die("trand", "Usage: trand([min,] max)\nDefault <min> is zero\n");
        RTExit(PARAM_ERROR);
	}
	if (min > max) {
		double tmp = max;
		max = min;
		min = tmp;
	}
	if (min < 0.0)
		min += 0.0000000001;
	if (max > 0.0)
		max -= 0.0000000001;

	int trunc = (int) _irand(min, max);
	return (double) trunc;
}

double m_chance(float p[], int n_args, double pp[])
{
    if (n_args != 2) {
		die("chance", "Usage: chance(num_rolls, num_sides)\n");
        RTExit(PARAM_ERROR);
    }
	float numer = p[0];
	float denom = p[1];
	if (denom == 0.0f)
		return 0.0;
	if (numer > denom)
		numer = denom;
	float rval = m_random();
	return rval <= numer / denom;
}

}	// end extern "C"

//-----------------------------------------------------------------------------
// C++ functions

extern "C" {
	double m_translen(const Arg args[], const int nargs);
	double m_pickrand(const Arg args[], const int nargs);
	double m_pickwrand(const Arg args[], const int nargs);
	double get_time(); // returns number of seconds that have elapsed
    double m_stringtofloat(const Arg args[], const int nargs);
}

#include "PField.h"

/* get the transposed length for a given input length at interval or series of intervals */
/* p0 = orig len   p1 = interval in octave point pc */
double m_translen(const Arg args[], const int nargs)
{
	if (!args[0].isType(DoubleType) || nargs != 2) {
		die("translen", "usage: translen(orig_length, transp), where 'orig_length' is a float and 'transp' is float, array, or table in pch format");
        RTExit(PARAM_ERROR);
	}
	double origLen = (double) args[0];
	double newLen = 0.0;
	
	if (args[1].isType(ArrayType)) {
		Array *a = args[1];
		const int alen = a->len;
		const double *adata = a->data;
		double intervalSum = 0.0;
		for (int j = 0; j < alen; j++) {
			double interval = octpch(adata[j]);
			intervalSum += interval;
		}
		intervalSum /= (double) alen;	// compute average
		double ratio = 1.0 / (cpsoct(10.0 + intervalSum) / cpsoct(10.0));
		newLen = origLen * ratio;
	}
	else if (args[1].isType(HandleType)) {
		Handle handle = (Handle) args[1];
		if (handle != NULL) {
			if (handle->type == PFieldType) {
				if (handle->ptr == NULL)  {
					die("translen", "NULL table handle for arg 1!");
                    RTExit(PARAM_ERROR);
				}
				PField *pf = (PField *) handle->ptr;
				double intervalSum = 0.0;
				for (int j = 0; j < 100; j++) {
					double interval = octpch(pf->doubleValue(double(j)/100.0));
					intervalSum += interval;
				}
				intervalSum /= 100.0;	// compute average
				double ratio = 1.0 / (cpsoct(10.0 + intervalSum) / cpsoct(10.0));
				newLen = origLen * ratio;
			}
			else {
				die("translen", "arg1 Handle can only be a table!");
                RTExit(PARAM_ERROR);
			}
		}
		else {
			die("translen", "NULL handle for arg 1!");
            RTExit(PARAM_ERROR);
		}
	}
	else if (args[1].isType(DoubleType)) {
		// This is exactly how TRANS does it, to assure a match.
		double interval = octpch((double) args[1]);
		newLen = origLen / (cpsoct(10.0 + interval) / cpsoct(10.0));
	}
	return newLen;
}

// pickrand returns random choice from its arguments
double m_pickrand(const Arg args[], const int nargs) 
{
    if (nargs == 0) {
		die("pickrand", "Must have at least one value to choose from!");
        RTExit(PARAM_ERROR);
    }

//FIXME: would be nice to have the array-unpacking in a function for
// use by others, instead of cutting/pasting it into each function. -JG
	// Load all args, including ones from flattened arrays, into xargs.
	double xargs[MAXDISPARGS];
	int nxargs = 0;
	for (int i = 0; i < nargs; i++) {
		if (args[i].isType(ArrayType)) {
			// NB: currently if we get this far, the array must contain only
			// doubles; otherwise error (e.g., in src/parser/minc/callextfunc.cpp).
			Array *a = args[i];
			const int alen = a->len;
			const double *adata = a->data;
			for (int j = 0; j < alen; j++) {
				xargs[nxargs++] = adata[j];
                if (nxargs == MAXDISPARGS) {
					die("pickrand", "Exceeded maximum number of arguments (%d)", MAXDISPARGS);
                    RTExit(PARAM_ERROR);
                }
			}
		}
		else if (args[i].isType(DoubleType)) {
			xargs[nxargs++] = args[i];
            if (nxargs == MAXDISPARGS) {
				die("pickrand", "Exceeded maximum number of arguments (%d)", MAXDISPARGS);
                RTExit(PARAM_ERROR);
            }
		}
        else {
			die("pickrand", "Arguments must be numbers or arrays of numbers");
            RTExit(PARAM_ERROR);
        }
	}
	float rindex;
	rindex = (m_random() * nxargs) - 0.000001; // 0 to 1.9999 for 2 args
	return xargs[(int) rindex];
}

// pickwrand returns choice based on <value, probability> pairs
double m_pickwrand(const Arg args[], const int nargs) 
{
    if (nargs == 0 || (nargs & 1)) {
		die("pickwrand", "Arguments must be in <value, probability> pairs!");
        RTExit(PARAM_ERROR);
    }

	// Load all args, including ones from flattened arrays, into xargs.
	double xargs[MAXDISPARGS];
	int nxargs = 0;
	for (int i = 0; i < nargs; i++) {
		if (args[i].isType(ArrayType)) {
			// NB: currently if we get this far, the array must contain only
			// doubles; otherwise error (e.g., in src/parser/minc/callextfunc.cpp).
			Array *a = args[i];
			const int alen = a->len;
			const double *adata = a->data;
			for (int j = 0; j < alen; j++) {
				xargs[nxargs++] = adata[j];
                if (nxargs == MAXDISPARGS) {
					die("pickwrand", "Exceeded maximum number of arguments (%d)", MAXDISPARGS);
                    RTExit(PARAM_ERROR);
                }
			}
		}
		else if (args[i].isType(DoubleType)) {
			xargs[nxargs++] = args[i];
            if (nxargs == MAXDISPARGS) {
				die("pickrand", "Exceeded maximum number of arguments (%d)", MAXDISPARGS);
                RTExit(PARAM_ERROR);
            }
		}
        else {
			die("pickrand", "Arguments must be numbers or arrays of numbers");
            RTExit(PARAM_ERROR);
        }
	}

	// sum up chances
	float totalchance = 0, psum = 0;
	for (int n = 1; n < nxargs; n += 2)
		totalchance += xargs[n];
	const float rindex = m_random() * totalchance;
	for (int n = 1; n < nxargs; n += 2) {
		psum += xargs[n];
		if (rindex <= psum)
			return xargs[n - 1];
	}
	return xargs[nxargs - 1];
}

double get_time() {
	double tval = RTcmix::getElapsedFrames()/RTcmix::sr();
	return tval;
}

double m_stringtofloat(const Arg args[], const int nargs)
{
    if (nargs != 1 || !args[0].isType(StringType)) {
        die("stringtofloat", "Usage: stringtofloat(some_string_containing_a_number");
        RTExit(PARAM_ERROR);
    }
    const char *stringToScan = (const char *)args[0];
    const char *str = stringToScan;
    // pull off any starting non-number chars
    while (!(*str >= '0' && *str <= '9') && (*str != '-') && (*str != '+') && *str != '.') str++;
    float value = 0;
    int found = sscanf(str, "%f", &value);
    return (found == EOF) ? 0.0 : value;
}


#ifdef DOUGS_CODE

#include "RawDataFile.h"
#include <sndlib.h>
#include <ctype.h>

extern "C" {

static RawDataFile *DNAFile;

double m_open_dnafile(const Arg args[], const int nargs)
{
	if (args[0].isType(StringType)) {
		const char *filename = (const char *)args[0];
		DNAFile = new RawDataFile(filename);
		if (DNAFile->openFileRead() < 0) {
			delete DNAFile;
			DNAFile = NULL;
			return -1;
		}
	}
	return 0;
}

static unsigned
swap(unsigned ul) {
	return (ul >> 24) | ((ul >> 8) & 0xff00) | ((ul << 8) & 0xff0000) | (ul << 24);
}

double m_get_codon()
{
	char bases[4];
	if (DNAFile == NULL) {
		rterror("get_codon", "You haven't opened a DNA file yet!");
        RTExit(PARAM_ERROR);
	}
	if (DNAFile->read(bases, 3) != 0) {
		rtcmix_warn("get_codon", "Reached EOF on DNA file");
        RTExit(FILE_ERROR);
	}
	//	Normalize
	bases[0] = toupper(bases[0]);
	bases[1] = toupper(bases[1]);
	bases[2] = toupper(bases[2]);
	bases[3] = '\0';
	
	unsigned codon = 0;
	memcpy(&codon, bases, 4);
#if MUS_LITTLE_ENDIAN
	// Swap
	codon = swap(codon);
#endif
	switch (codon) {
		case 'TTT\0':
		case 'TTC\0':
			return 0;	// Phe
		case 'TTA\0':
		case 'TTG\0':
		case 'CTT\0':
		case 'CTC\0':
		case 'CTA\0':
		case 'CTG\0':
			return 1;	// Leu
		case 'ATT\0':
		case 'ATC\0':
		case 'ATA\0':
			return 2;	// Iso
		case 'ATG\0':
			return 3;	// Met
		case 'GTT\0':
		case 'GTC\0':
		case 'GTA\0':
		case 'GTG\0':
			return 4;	// Val
			
		case 'TCT\0':
		case 'TCC\0':
		case 'TCA\0':
		case 'TCG\0':
			return 5;	// Ser
		case 'CCT\0':
		case 'CCC\0':
		case 'CCA\0':
		case 'CCG\0':
			return 6;	// Pro
		case 'ACT\0':
		case 'ACC\0':
		case 'ACA\0':
		case 'ACG\0':
			return 7;	// Thr
		case 'GCT\0':
		case 'GCC\0':
		case 'GCA\0':
		case 'GCG\0':
			return 8;	// Ala
			
		case 'TAT\0':
		case 'TAC\0':
			return 9;	// Tyr
		case 'TAA\0':
		case 'TAG\0':
			return -1;	// Stop codons
		case 'CAT\0':
		case 'CAC\0':
			return 10;	// His
		case 'CAA\0':
		case 'CAG\0':
			return 11;	// Gln
		case 'AAT\0':
		case 'AAC\0':
			return 12;	// Asn
		case 'AAA\0':
		case 'AAG\0':
			return 13;	// Lys
		case 'GAT\0':
		case 'GAC\0':
			return 14;	// Asp
		case 'GAA\0':
		case 'GAG\0':
			return 15;	// Glu
		case 'TGT\0':
		case 'TGC\0':
			return 16;	// Cys
		case 'TGA\0':
			return -1;	// Stop codon
		case 'TGG\0':
			return 17;	// Try
		case 'CGT\0':
		case 'CGC\0':
		case 'CGA\0':
		case 'CGG\0':
			return 18;	// Arg
		case 'AGT\0':
		case 'AGC\0':
			return 5;	// Ser
		case 'AGA\0':
		case 'AGG\0':
			return 18;	// Arg
		case 'GGT\0':
		case 'GGC\0':
		case 'GGA\0':
		case 'GGG\0':
			return 20;	// Gly
			
		default:
			break;
	}
	return -1;
}

}

#endif /* DOUGS_CODE */
