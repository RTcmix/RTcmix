/* SGRAN2 - stochastic granular synthesizer
 
 Args:
        p0: outskip
        p1: dur
        p2: amp*
        p3: rateLow (seconds before new grain)*
        p4: rateMid*
        p5: rateHigh*
        p6: rateTight*
        p7: durLow (length of grain in seconds)*
        p8: durMid*
        p9: durHigh*
        p10: durTight*
        p11: freqLow*
        p12: freqMid*
        p13: freqHigh*
        p14: freqTight*
        p15: panLow (0 - 1.0)*
        p16: panMid*
        p17: panHigh*
        p18: panTight*
        p19: wavetable**
        p20: grainEnv**
        p21: grainLimit=1500 (optional)

        * may receive pfield values
        ** must be passed pfield maketables.
    Kieran McAuliffe, 2023.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <algorithm>
#include <PField.h>
#include <Instrument.h>
#include "SGRAN2.h"
#include <rt.h>
#include <rtdefs.h>
#include <iostream>
#include <vector>

#define MAXGRAINS 1500

SGRAN2::SGRAN2() : branch(0)
{
}



SGRAN2::~SGRAN2()
{
	if (!configured)
		return;
	for (size_t i = 0; i < grains->size(); i++)
	{
		delete (*grains)[i];
	}
	delete grains;
}


int SGRAN2::init(double p[], int n_args)
{

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
	      return die("SGRAN2", "Output must be mono or stereo.");

	if (n_args < 21)
		return die("SGRAN2", "21 arguments are required");
	else if (n_args > 22)
		return die("SGRAN2", "too many arguments");
	grainEnvLen = 0;
	wavetableLen = 0;
	amp = p[2];

	newGrainCounter = 0;

	// init tables
	wavetable = (double *) getPFieldTable(19, &wavetableLen);
	grainEnv = (double *) getPFieldTable(20, &grainEnvLen);

	if (n_args > 21)
	{
		grainLimit = p[21];
		if (grainLimit > MAXGRAINS)
		{
			rtcmix_advise("STGRAN2", "user provided max grains exceeds limit, lowering to 1500");
			grainLimit = MAXGRAINS;
		}
			
	}
	else
		grainLimit = MAXGRAINS;

	return nSamps();
}



int SGRAN2::configure()
{
	// make the needed grains, which have no values yet as they need to be set dynamically
	grains = new std::vector<Grain*>();
	// maybe make the maximum grain value a non-pfield enabled parameter // mmh  was this done?

	for (int i = 0; i < grainLimit; i++)
	{
		grains->push_back(new Grain());
	}

	configured = true;

	return 0;	// IMPORTANT: Return 0 on success, and -1 on failure. // mmh was this done?
}

double SGRAN2::prob(double low,double mid,double high,double tight)
        // Returns a value within a range close to a preferred value
                    // tightness: 0 max away from mid
                     //               1 even distribution
                      //              2+amount closeness to mid
                      //              no negative allowed
{
	double range, num, sign;

	range = (high-mid) > (mid-low) ? high-mid : mid-low;
	do {
	  	if (rrand() > 0.)
			sign = 1.;
		else  sign = -1.;
	  	num = mid + sign*(pow((rrand()+1.)*.5,tight)*range);
	} while(num < low || num > high);
	return(num);
}

// set new parameters and turn on an idle grain
void SGRAN2::resetgrain(Grain* grain)
{
	float freq = cpsmidi((float)prob(midicps(freqLow), midicps(freqMid), midicps(freqHigh), freqTight));
	float grainDurSamps = (float) prob(grainDurLow, grainDurMid, grainDurHigh, grainDurTight) * SR;
	float panR = (float) prob((double)panLow, (double)panMid, (double)panHigh, (double)panTight); // mmh cast double
	grain->waveSampInc = wavetableLen * freq / SR;
	grain->ampSampInc = ((float)grainEnvLen) / grainDurSamps;
	grain->currTime = 0;
	grain->isplaying = true;
	grain->wavePhase = 0;
	grain->ampPhase = 0;
	grain->panR = panR;
	grain->panL = 1.0 - panR; // separating these in RAM means fewer sample rate calculations
  
	(*grain).dur = (int)round(grainDurSamps);
	//std::cout<<"sending grain with freq : " << freq << " dur : " << grain->dur << " panR panLow panMid panHigh panTight " << panR << " " << panLow << " " << panMid << " "  << panHigh << " " << panTight <<"\n";

}

void SGRAN2::resetgraincounter()
{
	newGrainCounter = (int)round(SR * prob(grainRateVarLow, grainRateVarMid, grainRateVarHigh, grainRateVarTight));
}

// determine the maximum grains we need total.  Needs to be redone using ZE CALCULUS
int SGRAN2::calcgrainsrequired()
{
	return ceil(grainDurMid / (grainRateVarMid * grainRate)) + 1;
}


// update pfields
void SGRAN2::doupdate()
{
	double p[19];
	update(p, 19);
	amp =(float) p[2];

	grainDurLow = (double)p[7];
	grainDurMid = (double)p[8]; if (grainDurMid < grainDurLow) grainDurMid = grainDurLow;
	grainDurHigh = (double)p[9]; if (grainDurHigh < grainDurMid) grainDurHigh = grainDurMid;
	grainDurTight = (double)p[10];


	grainRateVarLow = (double)p[3];
	grainRateVarMid = (double)p[4]; if (grainRateVarMid < grainRateVarLow) grainRateVarMid = grainRateVarLow;
	grainRateVarHigh = (double)p[5]; if (grainRateVarHigh < grainRateVarMid) grainRateVarHigh = grainRateVarMid;
	grainRateVarTight = (double)p[6];

	freqLow = (double)p[11];
	freqMid = (double)p[12]; if (freqMid < freqLow) freqMid = freqLow;
	freqHigh = (double)p[13]; if (freqHigh < freqMid) freqHigh = freqMid;
	freqTight = (double)p[14];

	if (freqLow < 15.0)
		freqLow = cpspch(freqLow);

	if (freqMid < 15.0)
		freqLow = cpspch(freqLow);

	if (freqHigh < 15.0)
		freqLow = cpspch(freqLow);

	panLow = (double)p[15];
	panMid = (double)p[16]; if (panMid < panLow) panMid = panLow;
	panHigh = (double)p[17]; if (panHigh < panMid) panHigh = panMid;
	panTight = (double)p[18];

	// Ouput amplitude will eventually be adjusted here
	// grainsRequired = calcgrainsrequired();
	// amp /= grainsRequired;

}


int SGRAN2::run()
{
	float out[2];
	for (int i = 0; i < framesToRun(); i++) {
		if (--branch <= 0)
		{
		doupdate();
		branch = getSkip();
		}

		out[0] = 0;
		out[1] = 0;
		for (size_t j = 0; j < grains->size(); j++)
		{
			Grain* currGrain = (*grains)[j];
			if (currGrain->isplaying)
			{
				if (++(*currGrain).currTime > currGrain->dur)
				{
					currGrain->isplaying = false;
				}
				else
				{
					// should include an interpolation option at some point
					float grainAmp = oscili(1, currGrain->ampSampInc, grainEnv, grainEnvLen, &((*currGrain).ampPhase));
					float grainOut = oscili(grainAmp,currGrain->waveSampInc, wavetable, wavetableLen, &((*currGrain).wavePhase));
					out[0] += grainOut * currGrain->panL;
					out[1] += grainOut * currGrain->panR;
				}
			}
			// this is not an else statement so a grain can be potentially stopped and restarted on the same frame

			if ((newGrainCounter <= 0) && !currGrain->isplaying)
			{
				resetgraincounter();
				if (newGrainCounter > 0) // we don't allow two grains to be create o
					{resetgrain(currGrain);}
				else
					{newGrainCounter = 1;}

			}
		}

		// if all current grains are occupied, we skip this request for a new grain
		if (newGrainCounter <= 0)
		{
			resetgraincounter();
		}

		out[0] *= amp;
		out[1] *= amp;
		rtaddout(out);
		newGrainCounter--;
		increment();
	}

	// Return the number of frames we processed.

	return framesToRun();
}


Instrument *makeSGRAN2()
{
	SGRAN2 *inst = new SGRAN2();
	inst->set_bus_config("SGRAN2");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("SGRAN2", makeSGRAN2);
}
#endif

