// setup.C -- Other Minc routines used to configure an LPCPLAY session

#include <ugens.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "lp.h"
#include "DataSet.h"
#include <RTcmix.h>

static double lowthresh, highthresh;
static float maxdev;	// lpcplay, set from outside
static float perperiod = 1.0;	// lpcplay, set from outside
static float cutoff;	// amp cutoff level, lpcplay, set via lpcstuff
static float hnfactor = 1.0;	// harmonic count multiplier, set via lpcstuff
static float thresh, randamp, unvoiced_rate;
static float risetime, decaytime;	// enveloping; set externally
static bool  autoCorrect = false;	// whether to stabilize each frame as it runs

static const int maxDataSets = 64;

#define THRESH_UNSET (-1)

// For right now, datasets are created each time they are needed and are
//	not shared.

char	g_dataset_names[maxDataSets][80];	// open data set names
DataSet	*g_datasets[maxDataSets];			// open datasets
int		g_currentDataset = 0;

// The following three functions are called by LPCPLAY::init() to copy the
//	current set of parameters from the Minc environment into the LPCPLAY
//	instance.

int GetDataSet(DataSet **ppDataSet)
{
	*ppDataSet = g_datasets[g_currentDataset];
	return g_datasets[g_currentDataset] ? 1 : -1;
}

int 
GetLPCStuff(double *pHiThresh,
			double *pLowThresh,
			float *pThresh,
			float *pRandamp,
			bool *pUnvoiced_rate,
			float *pRisetime, float *pDecaytime,
			float *pAmpcutoff)
{
	// Set these here if not already set.
	if (highthresh == THRESH_UNSET)
		highthresh = thresh + 0.000001;
	if (lowthresh == THRESH_UNSET)
		lowthresh = thresh;
	*pHiThresh = highthresh;
	*pLowThresh = lowthresh;
	*pThresh = thresh;
	*pRandamp = randamp;
	*pUnvoiced_rate = unvoiced_rate;
	*pRisetime = risetime;
	*pDecaytime = decaytime;
	*pAmpcutoff = cutoff;
	return 1;
}
					   
int
GetConfiguration(float *pMaxdev,
				 float *pPerperiod,
				 float *pHnfactor,
				 bool  *pAutoCorrect)
{
	*pMaxdev = maxdev;
	*pPerperiod = perperiod;
	*pHnfactor = hnfactor;
	*pAutoCorrect = autoCorrect;
	return 1;
}

//	These functions are all Minc utilities

double dataset(float *p, int n_args, double *pp)
/* p1=dataset name, p2=npoles */
{
	int i, set;
	char *name=DOUBLE_TO_STRING(pp[0]);

	if (name == NULL) {
		rterror("dataset", "NULL file name");
		return -1;
	}

	// Search all open dataset slots for matching name
	for (set = 0; set < maxDataSets && strlen(g_dataset_names[set]); ++set) {
		if (strcmp(name, g_dataset_names[set]) == 0) {
			g_currentDataset = set;
			::rtcmix_advise("dataset", "Using already open dataset at slot %d", set);
			return g_datasets[g_currentDataset]->getFrameCount();
		}
	}
	if (set >= maxDataSets) {
		::rterror("dataset", "Maximum number of datasets exceeded");
		return -1;
	}

	// OK, this is a new set that we will put in a new slot

	g_currentDataset = set;

	strcpy(g_dataset_names[g_currentDataset],name);

	int npolesGuess = 0;
	if(n_args>1)	/* if no npoles specified, it will be retrieved from */
		npolesGuess= (int) p[1];	/* the header (if USE_HEADERS #defined) */

	DataSet *dataSet = new DataSet;
	
	int frms = dataSet->open(name, npolesGuess, RTcmix::sr());
	
	if (frms < 0)
	{
		if (dataSet->getNPoles() == 0) {
			::rterror("dataset",
				"For this file, you must specify the correct value for npoles in p[1].");
		}
		return -1;
	}

	::rtcmix_advise("dataset", "File has %d poles and %d frames.",
			dataSet->getNPoles(), frms);
	
	// Add to dataset list.
	g_datasets[g_currentDataset] = dataSet;

	dataSet->ref();	// Note:  For now, datasets are never destroyed during run.

	return (double) frms;
}

double lpcstuff(float *p, int n_args)
/* p0=thresh, p1=random amp, p2=unvoiced rate p3= rise, p4= dec, p5=thresh cutof*/
{
        risetime=.01; decaytime=.1;
        if(n_args>0) thresh=p[0];
        if(n_args>1) randamp=p[1];
        if(n_args>2) unvoiced_rate=p[2];
        if(n_args>3) risetime=p[3];
        if(n_args>4) decaytime=p[4];
        if(n_args>5) cutoff = p[5]; else cutoff = 0;
        ::rtcmix_advise("lpcstuff", "Adjusting settings for %s.",g_dataset_names[g_currentDataset]); 
        ::rtcmix_advise("lpcstuff", "Thresh: %g  Randamp: %g  EnvRise: %g  EnvDecay: %g",
			   thresh,randamp, risetime, decaytime);
#ifdef WHEN_UNVOICED_RATE_WORKING
        if(unvoiced_rate == 1)
			::rtcmix_advise("lpcstuff", "Unvoiced frames played at normal rate.");
        else
			::rtcmix_advise("lpcstuff", "Unvoiced frames played at same rate as voiced 'uns.");
#else
        if(unvoiced_rate == 1) {
			::rtcmix_advise("lpcstuff", "Unvoiced rate option not yet working.");
			unvoiced_rate = 0;
		}
#endif
	return 0;
}

double set_hnfactor(float *p, int n_args)
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

double freset(float *p, int n_args)
{
        perperiod = p[0];
        ::rtcmix_advise("freset", "Frame reinitialization reset to %f times per period.",
				perperiod);
		return perperiod;
}


double setdev(float *p, int n_args)
{
        maxdev = p[0];
		::rtcmix_advise("setdev", "pitch deviation set to %g Hz", maxdev);
		return maxdev;
}

double setdevfactor(float *p, int n_args)
{
		// LPCPLAY will treat negatives as a factor
        maxdev = -p[0];
		::rtcmix_advise("setdevfactor", "pitch deviation factor: %g", -maxdev);
		return -maxdev;
}

#ifdef MAXMSP
// BGG -- see BGGx note in LPCPLAY.cpp
extern int BRADSSTUPIDUNVOICEDFLAG;
#endif

double
set_thresh(float *p, int n_args)
{
	double log10();

#ifdef MAXMSP
// BGG --  see BGGx note in LPCPLAY.cpp. I just want plain unvoiced sound!
	if (p[0] == -1.0) BRADSSTUPIDUNVOICEDFLAG = 1;
	else BRADSSTUPIDUNVOICEDFLAG = 0;
#endif

	if(p[1] <= p[0]) {
		::rterror("set_thresh", "upper thresh must be >= lower!");
		return -1;
	}
	lowthresh = p[0];
	highthresh = p[1];
	thresh = highthresh;
	::rtcmix_advise("set_thresh",
		   "lower error threshold: %0.6f  upper error threshold: %0.6f",
			p[0], p[1]);
	return lowthresh;
}

double
use_autocorrect(float *p, int n_args)
{
	autoCorrect = (p[0] != 0.0f);
	::rtcmix_advise("autocorrect", "auto-frame-correction turned %s", 
			autoCorrect == 0.0 ? "off" : "on");
	return p[0];
}

extern "C" {
#ifndef MAXMSP
int profile()
{
	float p[9]; double pp[9];
	UG_INTRO("lpcstuff",lpcstuff);
	UG_INTRO("dataset",dataset);
	UG_INTRO("freset",freset);
	UG_INTRO("setdev",setdev);
	UG_INTRO("setdevfactor",setdevfactor);
	UG_INTRO("set_thresh",set_thresh);
	UG_INTRO("set_hnfactor",set_hnfactor);
	UG_INTRO("autocorrect",use_autocorrect);
	p[0]=SINE_SLOT; p[1]=10; p[2]=1024; p[3]=1;
	pp[0]=SINE_SLOT; pp[1]=10; pp[2]=1024; pp[3]=1;
	makegen(p,4,pp);  /* store sinewave in array SINE_SLOT */
	p[0]=ENV_SLOT; p[1]=7; p[2]=512; p[3]=0; p[4]=512; p[5]=1; 
	pp[0]=ENV_SLOT; pp[1]=7; pp[2]=512; pp[3]=0; pp[4]=512; pp[5]=1; 
	makegen(p,6,pp);
	maxdev=0;
	lowthresh = THRESH_UNSET;
	highthresh = THRESH_UNSET;
	return 0;
}
#endif

#ifdef MAXMSP
int LPCprof_called = 0;

int LPCprofile()
{
	float p[9]; double pp[9];

	if (LPCprof_called == 1) return 0;

	p[0]=SINE_SLOT; p[1]=10; p[2]=1024; p[3]=1;
	pp[0]=SINE_SLOT; pp[1]=10; pp[2]=1024; pp[3]=1;
	makegen(p,4,pp);  /* store sinewave in array SINE_SLOT */
	p[0]=ENV_SLOT; p[1]=7; p[2]=512; p[3]=0; p[4]=512; p[5]=1;
	pp[0]=ENV_SLOT; pp[1]=7; pp[2]=512; pp[3]=0; pp[4]=512; pp[5]=1;
	makegen(p,6,pp);
	maxdev=0;
	lowthresh = 0;
	highthresh = 1;
	LPCprof_called = 1;
	return 0;
}
#endif // MAXMSP

}
