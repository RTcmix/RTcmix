#include <globals.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <ugens.h>
#include <sfheader.h>
#include <maxdispargs.h>

#define ARRAY_SIZE 256
#define NUM_ARRAYS  32
double minc_array[NUM_ARRAYS][ARRAY_SIZE],minc_array_size[NUM_ARRAYS];


double m_boost(float p[])
{ return(boost(p[0])); }

double m_pchmidi(float p[])
{ return(pchmidi((int)p[0])); }

double m_cpsmidi(float p[])
{ return(cpspch(pchmidi(((int)p[0])))); }

double m_midipch(float p[])
{ /*printf("%f\n",midipch(p[0]));*/ return(midipch(p[0])); }

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

double m_rand()
{ float rrand(); return(rrand()); }

double m_random()
{ float rrand(); return((rrand() * 0.5) + 0.5); }

double m_srand(float p[])
{ srrand(p[0]); return 0.0; }

double m_time_beat(float p[])
{ float time_beat(float); return(time_beat(p[0])); }

double m_beat_time(float p[])
{ float beat_time(float); return(beat_time(p[0])); }

double m_trunc(float p[])
{ return((double)(int)(p[0])); }

double m_ampdb(float p[])
{ return(ampdb(p[0])); }

double m_dbamp(float p[])
{ return(dbamp(p[0])); }

double m_stringify(float p, short n_args, double pp[])
{
	/* coerces a string passed in from Minc in quotes in p[0]
	   to a 'floating point' pointer suitable for use in
	   further cmix calls */

	return(pp[0]);
}

double m_log(float p[], short n_args)
{
   double val;

   val = log10((double)p[0]);

   return(val);
}

double m_pow(float p[], short n_args, double pp[])
{
	double val;

	val = pow(pp[0], pp[1]);
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

double m_print(float p[], short n_args, double pp[])
{
	printf("Value = %10.8f\n", pp[0]);
	return 0.0;
}

double m_abs(float p[], int n_args)
{
	return((p[0] >= 0.0) ? p[0] : -(p[0]));
}

double m_mod(float p[], int n_args)
{
	int i,j;
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

double m_exit(float p[], int n_args)
{
	closesf();
	return 0.0;
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

double m_getpch(float p[], short n_args, double pp[])
{
	int pchfd;
	int frameno,nbframe,iname;
	long skipbytes;
	float vals[200]; /* enough for 46 poles + 4 data values */
	char  *input;

/*	p0=name of pchanal file; p1=framenumber to get */

	iname = (int)pp[0];
	input = (char *)iname;

	if((pchfd = open(input,0)) < 0)
		die("getpch", "Can't open pitch analysis file");

	nbframe = 2*FLOAT; 
	frameno = (int)p[1];

	skipbytes = frameno * nbframe;
	if (lseek(pchfd, skipbytes, 0) < 0)
		die("getpch", "Error on pchanal lseek");

	if (read(pchfd, (char *)vals, nbframe) != nbframe)
		die("getpch", "Bad read on pchanal file");

	close(pchfd);

	return((double) vals[0]);
}

double m_getamp(float p[], short n_args, double pp[])
{
	int pchfd;
	int frameno,nbframe,iname;
	long skipbytes;
	float vals[200]; /* enough for 46 poles + 4 data values */
	char  *input;

/*	p0=name of pchanal file; p1=framenumber to get */

	iname = (int)pp[0];
	input = (char *)iname;

	if((pchfd = open(input,0)) < 0)
		die("getamp", "Can't open pitch analysis file");

	nbframe = 2*FLOAT; 
	frameno = (int)p[1];

	skipbytes = frameno * nbframe;
	if (lseek(pchfd, skipbytes, 0) < 0)
		die("getamp", "Error on pchanal lseek");

	if (read(pchfd, (char *)vals, nbframe) != nbframe)
		die("getamp", "Bad read on pchanal file");

	close(pchfd);

	return((double) vals[1]);
}

double str_num(float p[], short n_args, double pp[])
{
	char *name;
	int i,j;
	char buf[16];

	for (j=0; j<n_args; j=j+2) {
		buf[0] = 0;
		name = 0;
		i = (int) pp[j];
		if (((j+1) < (n_args-1)) || !(n_args % 2))
			sprintf(buf, "%g", pp[j+1]);
		name = (char *) i;
		printf("%s%s", name, buf);
	}
	printf("\n");
	return 0.0;
}

double m_print_is_on(float p[], int n_args)
{
	print_is_on = 1;
	return print_is_on;
}

double m_print_is_off(float p[], int n_args)
{
	print_is_on = 0;
	return print_is_on;
}

struct slist slist[NUM_ARRAYS];
double m_get_spray(float p[], int n_args)
{
	int i = p[0];
	return((double)(spray(&slist[i])));
}

double m_spray_init(float p[], int n_args)
{
	int i,j;
	i=p[0]; j=p[1];
	sprayinit(&slist[i],j,p[2]);
	return 0.0;
}


static int line_array_size = 1000;      /* modified by m_setline_size */

double m_setline_size(float p[], int n_args)
{
	if (p[0] < 2)
		die("setline_size", "Setline array size must be at least 2!");
	line_array_size = p[0];
	advise("setline_size", "Setline arrays will have %d elements.",
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

	makegen(pp, n_args+3);

	return 0.0;
}


/* Note: MIX used 200 as default; WAVETABLE used 1000. It needs a faster
   rate, since it's changing an oscillator at that speed, rather than just
   an amplitude envelope. I'm just making 1000 the default for everyone.  -JGG
*/
int resetval = 1000;                 /* modified by m_reset; read by insts */

double m_reset(float p[], int n_args)
{
	if (p[0] <= 0)
		die("reset", "Reset value must be greater than 0!");
	resetval = p[0];
	advise("reset", "Envelope calls set to %d times per sec.", resetval);

	return 0.0;
}


