/* frequeny modulate an input sound -- thanks to Sam Fenster! */

#include <ugens.h>
#include <sfheader.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <math.h>

/*  wow -- apply fm to an input soundfile
*
*  p0 = output skip
*  p1 = input skip
*  p2 = duration
*  p3 = amplitude
*  p4 = modulator depth (index)
*  p5 = modulator frequency (hz)
*  p6 = input channel [optional]
*  p7 = stereo spread <0-1> [optional]
*  assumes function table 1 is the amplitude envelope
*  function table 2 is modulator waveform
*
*/

extern SFHEADER      sfdesc[NFILES];
extern float SR();

#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))

double
wow(float p[], int n_args)
{
	int n,nsamps,msize;
	int i,j,input_offset_floor;
	int q_size, q_past, q_future, q_middle, q_end, q_pos, q_pos1;
	int inchan,inchans,outchans;
	int skip;

	float input_start, output_start, duration;
	double *modulator; 
	float mphase, modulator_index; 
	float modulator_normalize, modulator_freq, amp, input_offset, si;
	float *queue, min_x, max_x, integral, out[2], weight;
	float amptabs[2];
	double *amparr;
	float aamp;

	inchans = sfchans(&sfdesc[0]);
	outchans = sfchans(&sfdesc[1]);

	input_start = p[1] * SR();     /* All measured in # of samples */
	output_start = p[0] * SR();
	duration =  p[2] * SR();

	amp = aamp = p[3];

	amparr = floc(1);
	if (amparr) {
		int lenamp = fsize(1);
		tableset(SR(), p[2], lenamp, amptabs);
	}
	else
		rtcmix_advise("wow", "Setting phrase curve to all 1's.");

	modulator_index = p[4];
	modulator_freq = p[5];

	modulator = floc(2);       /* Pointer to start of gen array */
	if (modulator == NULL)
		die("wow", "You haven't made the modulator waveform (table 2).");
	msize = fsize(2); /* Size of gen array */

/* ---------------------------------------------------------------------- */
/* We would like to find the average of gen array 1, so we can subtract it
      to normalize the modulator (so its integral is zero). */

	nsamps = setnote(output_start/SR(), duration/SR(), 1);
	integral = 0;
	for (i = 0; i < msize; i++) integral += modulator[i];

	modulator_normalize = modulator_index * (integral/(float)msize);

/* Also we'd like to find the max and min of the integral, so we'll know
      how many samples in front of and behind the current one we need to
      store. */

	mphase = 0;
	si = (float)msize * modulator_freq/SR();

	integral = min_x = max_x = 0;
	for (n = 0; n < nsamps; n++) {
		integral += oscili(modulator_index, si, modulator, 
			msize, &mphase) - modulator_normalize;

		if (integral < min_x) min_x = integral;
		if (integral > max_x) max_x = integral;
		}

	q_past = -floor (min_x);
	q_past = MAX (q_past, 0);
	q_future = ceil (max_x);
	q_future = MAX (q_future, 0);

printf ("wow: Min offset (samples): %d.  Max: %d.\n", -q_past, q_future);
printf ("     Avg of modulator: %f.  Max of integral: %f.  Min: %f.\n",
		modulator_normalize, max_x, min_x);

	q_size = q_past + q_future + 1;
	q_size *= inchans;
/*	queue = (float *)calloc(q_size, sizeof (float)); */
	queue = (float *)valloc(q_size*sizeof(float));
	if (!queue) {
		fprintf (stderr, "wow: Out of memory for queue.\n");
		return 0;
		}

	input_start -= q_past;
	i = -MIN (input_start, 0);
	input_start = MAX(input_start, 0);
	duration += (q_size/inchans) - 1;
	setnote(input_start/SR(), duration/SR(), 0);

	for (i; i < q_size; i += inchans) GETIN (queue+i, 0);

	q_middle = q_past;
	q_end = q_middle + q_future;

/* ---------------------------------------------------------------------- */
/* Enough setting up.  Let's get down to it. */

	skip = SR()/(float)resetval;
	inchan = p[6];
	mphase = 0;
	input_offset = 0;
	j = 0;
	for (n = 0; n < nsamps; n++) {
		while (!j--) {
			if (amparr)
				aamp = tablei(n,amparr,amptabs) * amp;
			j = skip;
			}

		input_offset += oscili (modulator_index, si, modulator,
			msize, &mphase) - modulator_normalize;

		input_offset_floor = floor (input_offset);
		weight = input_offset - input_offset_floor;
		q_pos = q_middle + input_offset_floor;
		q_pos *= inchans;
		q_pos1 = q_pos + inchans;
		if (q_pos > q_size) q_pos -= q_size;
		if (q_pos1 > q_size) q_pos1 -= q_size;
		out[0] = queue[q_pos+inchan]*(1-weight) + queue[q_pos1+inchan]*weight;
		out[0] *= aamp;
		if (outchans > 1) {
			out[1] = (1.0 - p[7]) * out[0];
			out[0] *= p[7];
			}
		ADDOUT (out, 1);

		q_middle = q_middle + 1;
		if ((q_middle*inchans) == q_size) q_middle = 0;
		q_end = q_end + 1;
		if ((q_end*inchans) == q_size) q_end = 0;
		GETIN(queue+(q_end*inchans),0);
		}

	endnote(1);
	free(queue);

	return 0.0;
}




#if 0
Here are some debugging statements which can be inserted in the code above,
if desired:
-------------------------------------------------------------------------------

   printf ("wow: Min offset (samples): %d.  Max: %d.\n", -q_past, q_future);
   printf ("     Avg of modulator: %f.  Max of integral: %f.  Min: %f.\n",
	   modulator_normalize, max_x, min_x);

-------------------------------------------------------------------------------

      if (input_offset > q_future  ||  input_offset < -q_past)
	 {fprintf (stderr, "wow: Fluctuated too far.\n");
	  fprintf (stderr,
		   "     at sample %d, modulator loc %f, offset = %f.\n",
		   n, mphase, input_offset);
	  return 0;}

-------------------------------------------------------------------------------
#endif


int profile()
{
	UG_INTRO("wow",wow);

	return 0;
}
