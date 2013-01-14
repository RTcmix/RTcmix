#include <stddef.h>	// for NULL
#include <ugens.h>
#include "cfuncs.h"

typedef struct {
	float cf;
	float bw;
	float amp;
} FilterSpec;

static FilterSpec _filter_spec[MAXFILTER];
static int _num_filters = 0;

/* setup filters for IIR instruments

   p0 - center frequency
        if < 15, indicates oct.pc format instead of Hz
        if negative, abs. value is multiplier of first frequency given
   p1 - bandwidth (Hz)
        if negative, abs. value is percent (from 0 to 1) of frequency
   p2 - amplitude multiplier

   followed by additional triplets for a total of 64 filters.
*/
double IIR_setup(float p[], int n_args)
{
	int i, j;
	float first = 1.0f;

	rtcmix_advise("IIR setup", "centerfreq    bandwidth  relative amp");

	for (i = 0, j = 0; i < n_args; i += 3, j++)  {
		float cf, bw, amp;

		if (j == MAXFILTER)
			return die("IIR setup", "Can't have more than %d filters.", MAXFILTER);

		if (p[i] < 0.0)
			cf = -p[i] * first;
		else
			cf = (p[i] < 15.0) ? cpspch(p[i]) : p[i];
		if (i == 0)
			first = cf;
		bw = p[i + 1] < 0.0 ? -p[i + 1] * cf : p[i + 1];
		amp = p[i + 2];

		_filter_spec[j].cf = cf;
		_filter_spec[j].bw = bw;
		_filter_spec[j].amp = amp;

		rtcmix_advise(NULL, "            %10.4f %10.4f %10.4f", cf, bw, amp);
	}
	_num_filters = j;
	return (double) _num_filters;
}


/* Retrieve current descriptions of up to MAXFILTER filters.  Returns the
   actual number of filters, and fills in the <cf>, <bw> and <amp> arrays
   appropriately.  These arrays must be dimensioned by caller to MAXFILTER!
   Returns 0 if script has not called setup yet (or has called it with no args).
*/
int get_iir_filter_specs(float cf[MAXFILTER], float bw[MAXFILTER],
                                                        float amp[MAXFILTER])
{
	int i;

	for (i = 0; i < _num_filters; i++) {
		cf[i] = _filter_spec[i].cf;
		bw[i] = _filter_spec[i].bw;
		amp[i] = _filter_spec[i].amp;
	}
	return _num_filters;
}

#ifndef MAXMSP
int profile()
{
	UG_INTRO("setup", IIR_setup);
	return 0;
}
#endif
