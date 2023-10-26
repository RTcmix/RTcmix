// setup.C -- Other Minc routines used to configure an LPCPLAY session

#include <ugens.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "setup.h"
#include "lp.h"
#include "LPCDataSet.h"
#include <RTcmix.h>

#define THRESH_UNSET (-1)
static const float kDefaultSmoothingFactor = 0.5;

static double lowthresh = THRESH_UNSET, highthresh = THRESH_UNSET;
static float maxdev = 0;	// lpcplay, set from outside
static float perperiod = 1.0;	// lpcplay, set from outside
static float cutoff;	// amp cutoff level, lpcplay, set via lpcstuff
static float hnfactor = 1.0;	// harmonic count multiplier, set via lpcstuff
static float randamp, unvoiced_rate;
static float risetime, decaytime;	// enveloping; set externally
static bool  autoCorrect = false;	// whether to stabilize each frame as it runs
static float sourceDuration = 0.0;  // duration of source used to create LPC frames (passed in)

static float pitchFixOctave = -1.0;  // -1 indicates don't fix; 0 indicates no target octave
static bool fixPitchGaps = false;
static float pitchSmoothingFactor = 0.0;    // 0 indicates no smoothing

static const int maxLPCDataSets = 64;

// For right now, datasets are created each time they are needed and are
//	not shared.

char	g_dataset_names[maxLPCDataSets][80];	// open data set names
LPCDataSet	*g_datasets[maxLPCDataSets];			// open datasets
int		g_currentDataset = 0;

static void resetPitchPreprocessing()
{
    ::rtcmix_advise("lpcstuff", "Resetting thresh, deviation, etc.");
    maxdev = 0.0;
    sourceDuration = 0.0;  // duration of source used to create LPC frames (passed in)

    lowthresh = THRESH_UNSET;
    highthresh = THRESH_UNSET;

    pitchFixOctave = -1.0;  // -1 indicates don't fix; 0 indicates no target octave
    fixPitchGaps = false;
    pitchSmoothingFactor = 0.0;    // 0 indicates no smoothing
}

// The following three functions are called by LPCPLAY::init() to copy the
//	current set of parameters from the Minc environment into the LPCPLAY
//	instance.

int GetLPCDataSet(LPCDataSet **ppLPCDataSet)
{
	*ppLPCDataSet = g_datasets[g_currentDataset];
	return g_datasets[g_currentDataset] ? 1 : -1;
}

int 
GetLPCStuff(double *pHiThresh,
			double *pLowThresh,
			float *pRandamp,
			bool *pUnvoiced_rate,
			float *pRisetime, float *pDecaytime,
			float *pAmpcutoff,
            float *pSourceDuration)
{
	// Set these here if not already set -- but this should now be impossible.
	if (highthresh == THRESH_UNSET)
		highthresh = 1.0;
	if (lowthresh == THRESH_UNSET)
		lowthresh = 0.0;
	*pHiThresh = highthresh;
	*pLowThresh = lowthresh;
	*pRandamp = randamp;
	*pUnvoiced_rate = unvoiced_rate != 0.0;
	*pRisetime = risetime;
	*pDecaytime = decaytime;
	*pAmpcutoff = cutoff;
    *pSourceDuration = sourceDuration;
	return 1;
}
					   
int
GetConfiguration(float *pMaxdev,
				 float *pPerperiod,
				 float *pHnfactor,
				 bool  *pAutoCorrect,
                 float *pPitchFixOctave,
                 bool *pFixGaps,
                 float *pSmoothingFactor)
{
	*pMaxdev = maxdev;
	*pPerperiod = perperiod;
	*pHnfactor = hnfactor;
	*pAutoCorrect = autoCorrect;
    *pPitchFixOctave = pitchFixOctave;
    *pFixGaps = fixPitchGaps;
    *pSmoothingFactor = pitchSmoothingFactor;
	return 1;
}

//	These functions are all Minc utilities

double dataset(double p[], int n_args)
/* p1=dataset name, p2=npoles */
{
	int set;
	char *name=DOUBLE_TO_STRING(p[0]);

	if (name == NULL) {
        ::rterror("dataset", "NULL file name");
		return rtOptionalThrow(PARAM_ERROR);
	}

	// Search all open dataset slots for matching name
	for (set = 0; set < maxLPCDataSets && strlen(g_dataset_names[set]); ++set) {
		if (strcmp(name, g_dataset_names[set]) == 0) {
			g_currentDataset = set;
			::rtcmix_advise("dataset", "Using already open dataset at slot %d", set);
			return g_datasets[g_currentDataset]->getFrameCount();
		}
	}
	if (set >= maxLPCDataSets) {
		::rterror("dataset", "Maximum number of datasets exceeded");
		return rtOptionalThrow(SYSTEM_ERROR);
	}

	int npolesGuess = 0;
	if(n_args>1)	/* if no npoles specified, it will be retrieved from */
		npolesGuess= (int) p[1];	/* the header */

	LPCDataSet *dataSet = new LPCDataSet;
	
	int frms = (int) dataSet->open(name, npolesGuess, RTcmix::sr());
	
	if (frms < 0) {
		if (dataSet->getNPoles() == 0) {
			::rterror("dataset",
				"For this file, you must specify the correct value for npoles in p[1].");
		}
		return rtOptionalThrow(PARAM_ERROR);
	}

	::rtcmix_advise("dataset", "File has %d poles and %d frames.",
			dataSet->getNPoles(), frms);
	
    // OK, now put put in a new slot

    g_currentDataset = set;

    strcpy(g_dataset_names[g_currentDataset],name);

	// Add to dataset list.
	g_datasets[g_currentDataset] = dataSet;

	dataSet->ref();	// Note:  For now, datasets are never destroyed during run.
    
    // Reset any state which would not make sense with a new LPC file

	return (double) frms;
}

double lpcstuff(double *p, int n_args)
/* p0=thresh, p1=random amp, p2=unvoiced rate p3= rise, p4= dec, p5=thresh cutof, p6=source duration*/
{
        risetime=.01f; decaytime=.1f;
        resetPitchPreprocessing();
        if (n_args>0) {
            // high thresh must be > than lowthresh, which must be > 0
            float thresh=std::max(0.0, p[0]);
            lowthresh = thresh;
            highthresh = thresh + 0.00001;
        }
        if (n_args>1) randamp=p[1];
        if (n_args>2) unvoiced_rate=p[2];
        if (n_args>3) risetime=p[3];
        if (n_args>4) decaytime=p[4];
        if (n_args>5) cutoff = p[5];
        else cutoff = 0;
        if (n_args>6) sourceDuration = p[6];
        else sourceDuration = 0.0;
        ::rtcmix_advise("lpcstuff", "Adjusting settings for %s.",g_dataset_names[g_currentDataset]);
        ::rtcmix_advise("lpcstuff", "Thresh: %g  Randamp: %g  EnvRise: %g  EnvDecay: %g",
                        highthresh,randamp, risetime, decaytime);
        if (unvoiced_rate == 1) {
#ifdef FAIL_IF_DURATION_MISSING     /* not necessary in current code */
            if (sourceDuration > 0.0)
#endif
                ::rtcmix_advise("lpcstuff", "Unvoiced frames played at normal rate.");
#ifdef FAIL_IF_DURATION_MISSING     /* not necessary in current code */
            else {
                ::rterror("lpcstuff",
                          "To play unvoiced frames at normal rate, you must specify the original source duration in p[6]");
                unvoiced_rate = 0;
                return rtOptionalThrow(PARAM_ERROR);
            }
#endif
        }
        else {
			::rtcmix_advise("lpcstuff", "Unvoiced frames played at same rate as voiced 'uns.");
        }
	return 1;
}

double set_hnfactor(double *p, int n_args)
{
	if (p[0] < .01)
	{
		rtcmix_warn("set_hnfactor", "hnfactor must be greater than 0.01...ignoring");
		return hnfactor;
	}
	hnfactor = p[0];
	::rtcmix_advise("set_hnfactor", "Harmonic count factor set to %g", hnfactor);
	return p[0];
}

double freset(double *p, int n_args)
{
        perperiod = p[0];
        ::rtcmix_advise("freset", "Frame reinitialization reset to %f times per period.",
				perperiod);
		return perperiod;
}


double setdev(double *p, int n_args)
{
        maxdev = p[0];
		::rtcmix_advise("setdev", "pitch deviation set to %g Hz", maxdev);
		return maxdev;
}

double setdevfactor(double *p, int n_args)
{
		// LPCPLAY will treat negatives as a factor
        maxdev = -p[0];
		::rtcmix_advise("setdevfactor", "pitch deviation factor: %g", -maxdev);
		return -maxdev;
}

// Set the threshold below which the frame will be 100% voiced, and the threshold above which
// the frame will be 100% unvoiced.  This needs to be reset after each call to lpcstuff()

double
set_thresh(double *p, int n_args)
{
	if(p[1] <= p[0]) {
		::rterror("set_thresh", "upper thresh must be > lower!");
        return rtOptionalThrow(PARAM_ERROR);
	}
	lowthresh = p[0];
	highthresh = p[1];
	::rtcmix_advise("set_thresh",
		   "lower error threshold: %0.6f  upper error threshold: %0.6f",
			p[0], p[1]);
	return lowthresh;
}

double
use_autocorrect(double *p, int n_args)
{
	autoCorrect = (p[0] != 0.0f);
	::rtcmix_advise("autocorrect", "auto-frame-correction turned %s", 
			autoCorrect == 0.0 ? "off" : "on");
	return p[0];
}

// use_fix_pitch_octaves(p0 = true/false [, p1 = target octave in pch])

double use_fix_pitch_octaves(double *p, int n_args)
{
    if (p[0] == 0.0) {
        pitchFixOctave = -1.0;
        ::rtcmix_advise("fix_pitch_octaves", "disabled fixing");
    }
    else {
        if (n_args >= 2) {
            if (p[1] < 20.0) {
                pitchFixOctave = (float)cpspch(p[1]);
                ::rtcmix_advise("fix_pitch_octaves", "using %.2f (%f Hz) as target starting pitch", p[1], pitchFixOctave);
            }
            else {
                pitchFixOctave = p[1];
                ::rtcmix_advise("fix_pitch_octaves", "using %f Hz as target starting pitch", pitchFixOctave);
            }
        }
        else {
            pitchFixOctave = 0.0;
            ::rtcmix_advise("fix_pitch_octaves", "enabled with no target starting pitch");
        }
    }
    return p[0];
}

double use_fix_pitch_gaps(double *p, int n_args)
{
    fixPitchGaps = p[0] != 0.0;
    return 0;
}

double use_pitch_smoothing(double *p, int n_args)
{
    if (p[0] <= 0.0) {
        pitchSmoothingFactor = 0.0;
        ::rtcmix_advise("use_pitch_smoothing", "disabled smoothing");
    }
    else {
        if (n_args >= 2) {
            if (p[1] >= 1.0) {
                rtcmix_warn("use_pitch_smoothing", "factor must be less than 1.0... ignoring");
            }
            else {
                pitchSmoothingFactor = p[1];
            }
        }
        else {
            pitchSmoothingFactor = kDefaultSmoothingFactor;
        }
        ::rtcmix_advise("use_pitch_smoothing", "using factor of %f", pitchSmoothingFactor);
    }
    return 0.0;
}

extern "C" {
#ifndef EMBEDDED
int profile()
{
	double p[9];
	UG_INTRO("lpcstuff",lpcstuff);
	UG_INTRO("dataset",dataset);
	UG_INTRO("freset",freset);
	UG_INTRO("setdev",setdev);
	UG_INTRO("setdevfactor",setdevfactor);
	UG_INTRO("set_thresh",set_thresh);
	UG_INTRO("set_hnfactor",set_hnfactor);
	UG_INTRO("autocorrect",use_autocorrect);
    UG_INTRO("fix_pitch_octaves",use_fix_pitch_octaves);
    UG_INTRO("fix_pitch_gaps",use_fix_pitch_gaps);
    UG_INTRO("pitch_smoothing",use_pitch_smoothing);
	p[0]=SINE_SLOT; p[1]=10; p[2]=1024; p[3]=1;
	makegen(p,4);  /* store sinewave in array SINE_SLOT */
	p[0]=ENV_SLOT; p[1]=7; p[2]=512; p[3]=0; p[4]=512; p[5]=1; 
	makegen(p,6);
	return 0;
}
#endif

#ifdef EMBEDDED
int LPCprof_called = 0;

int LPCprofile()
{
	double p[9];

	if (LPCprof_called == 1) return 0;

	p[0]=SINE_SLOT; p[1]=10; p[2]=1024; p[3]=1;
	makegen(p,4);  /* store sinewave in array SINE_SLOT */
	p[0]=ENV_SLOT; p[1]=7; p[2]=512; p[3]=0; p[4]=512; p[5]=1;
	makegen(p,6);
	LPCprof_called = 1;
	return 0;
}
#endif // EMBEDDED

}
