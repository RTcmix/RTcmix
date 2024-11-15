/* STGRAN2 - stochastic granular synthesizer
 processes soundfile or live input signal.
 
Args:
    p0: outskip
    p1: inskip
    p2: dur
    p3: amp*
    p4: rateLow (seconds before new grain)*
    p5: rateMid*
    p6: rateHigh*
    p7: rateTight*
    p8: durLow (length of grain in seconds)*
    p9: durMid*
    p10: durHigh*
    p11: durTight*
    p12: transLow (oct.pc)*
    p13: transMid (oct.pc)*
    p14: transHigh (oct.pc)*
    p15: transTight*
    p16: panLow (0 - 1.0)*
    p17: panMid*
    p18: panHigh*
    p19: panTight*
    p20: grainEnv**
    p21: bufferSize=1 (size of the buffer used to choose new grains)*
    p22: maxGrains=1500
    p23: inchan=0
    
    * may receive a table or some other pfield source
    ** must be passed as a pfield maketable.
    Kieran McAuliffe, 2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <algorithm>
#include <PField.h>
#include <Instrument.h>
#include "STGRAN2.h"		// declarations for this instrument class
#include <rt.h>
#include <rtdefs.h>
#include <iostream>
#include <vector>

#define MAXBUFFER 441000
#define MAXGRAINS 1500

AUDIOBUFFER::AUDIOBUFFER(int size): _full(false), _head(0)
{
    _buffer = new std::vector<double>(MAXBUFFER);
	_size = size;
	// size refers to the PREFERRED size
}

AUDIOBUFFER::~AUDIOBUFFER()
{
    delete _buffer;
}

void AUDIOBUFFER::Append(double sample)
{
    (*_buffer)[_head] = sample;
    if (_head == _buffer->size() -1)
		_full = true;
    _head = (_head + 1) % _buffer->size();
}

double AUDIOBUFFER::Get(float index)
{
	while (index < 0)
		index += (float) _buffer->size();
	while (index > _buffer->size())
		index -= (float) _buffer->size();
	int i = (int) index;
	int k = i + 1;
	float frac = index - i;

    return (*_buffer)[i] + ((*_buffer)[k] - (*_buffer)[i]) * frac;
}

int AUDIOBUFFER::GetHead()
{
    return _head;
}

int AUDIOBUFFER::GetSize()
{
    return _size;
}

int AUDIOBUFFER::GetMaxSize()
{
	return _buffer->size();
}

void AUDIOBUFFER::SetSize(int size)
{
	_size = size;
}

bool AUDIOBUFFER::GetFull()
{
    return _full;
}

bool AUDIOBUFFER::CanRun()
{
	return (GetFull() || GetHead() > GetSize());
}

void AUDIOBUFFER::Print()
{
    for (size_t i = 0; i < _buffer->size(); i++)
    {
        std::cout << (*_buffer)[i] << " ,";
    }
    std::cout << "\n";
}

STGRAN2::STGRAN2() : branch(0), configured(false)
{
}


// Destruct an instance of this instrument, freeing any memory you've allocated.

STGRAN2::~STGRAN2()
{
	//std::cout << " grains used " << grainsUsed << "\n";
	if (!configured)
		return;
	for (size_t i = 0; i < grains->size(); i++)
	{
		delete (*grains)[i];
	}
	delete grains;
	delete buffer;
	delete in;
}



int STGRAN2::init(double p[], int n_args)
{


	if (rtsetinput(p[1], this) == -1)
      		return DONT_SCHEDULE; // no input

	if (rtsetoutput(p[0], p[2], this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
	      return die("STGRAN2", "Output must be mono or stereo.");

	if (n_args < 21)
		return die("STGRAN2", "21 arguments are required");

	else if (n_args > 24)
		return die("STGRAN2", "too many arguments");

	if (inputChannels() > 1)
		rtcmix_advise("STGRAN2", "Only the first input channel will be used");

	_nargs = n_args;

	grainEnvLen = 0;
	amp = p[3];

	if (n_args > 22)
	{
		grainLimit = p[22];
		if (grainLimit > MAXGRAINS)
		{
			rtcmix_advise("STGRAN2", "user provided max grains exceeds limit, lowering to 1500");
			grainLimit = MAXGRAINS;
		}
			
	}
	else
		grainLimit = MAXGRAINS;

	if (n_args > 23)
		inchan = p[23];
	else
		inchan = 0;

	if (inchan >= inputChannels())
		return die("MYINST", "You asked for channel %d of a %d-channel input.",								                                             inchan, inputChannels());

	newGrainCounter = 0;

	// init tables
	grainEnv = (double *) getPFieldTable(20, &grainEnvLen);

	oneover_cpsoct10 = 1.0 / cpsoct(10.0);

	return nSamps();
}


int STGRAN2::configure()
{
	// make the needed grains, which have no values yet as they need to be set dynamically
	grains = new std::vector<Grain*>();
	// maybe make the maximum grain value a non-pfield enabled parameter
	for (int i = 0; i < grainLimit; i++)
	{
		grains->push_back(new Grain());
	}

	buffer = new AUDIOBUFFER(MAXBUFFER);

	in = new float[RTBUFSAMPS*inputChannels()];

	configured = true;

	return 0;	// IMPORTANT: Return 0 on success, and -1 on failure.
}

double STGRAN2::prob(double low,double mid,double high,double tight)
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
void STGRAN2::resetgrain(Grain* grain)
{

	if (!buffer->CanRun())
		return;

	float trans = (float)prob(transLow, transMid, transHigh, transTight);
	float increment = cpsoct(10.0 + trans) * oneover_cpsoct10;
	float offset = increment - 1;
	float grainDurSamps = (float) prob(grainDurLow, grainDurMid, grainDurHigh, grainDurTight) * SR;
	int sampOffset = (int) round(abs(grainDurSamps * offset)); // how many total samples the grain will deviate from the normal buffer movement

	if (sampOffset >= buffer->GetMaxSize()) // this grain cannot exist with size of the buffer
	{
		rtcmix_advise("STGRAN2", "GRAIN IGNORED, TRANSPOSITION OR DURATION TOO EXTREME");
		return;
	}

	else if ((sampOffset >= buffer->GetSize()) && (offset > 0))
	{
		// shift this grain
		grain->currTime = buffer->GetHead() - sampOffset;
	}
	else
	{
		int minShift;
		int maxShift;

		if (offset > 0)
		{
			minShift = sampOffset;
			maxShift = buffer->GetSize();
		}
		else
		{
			minShift = 1;
			maxShift = buffer->GetSize() - sampOffset;
		}

		if (maxShift == minShift)
		{
			return; // There's a better way to handle this that I'll add at some point...
		}
		
		grain->currTime = buffer->GetHead() - (rand() % (maxShift - minShift) + minShift);
		
	}

	
	
	float panR = (float) prob(panLow, panMid, panHigh, panTight);
	grain->waveSampInc = increment;
	grain->ampSampInc = ((float)grainEnvLen) / grainDurSamps;

	grain->isplaying = true;
	grain->ampPhase = 0;
	grain->panR = panR;
	grain->panL = 1 - panR; // separating these in RAM means fewer sample rate calculations
	grain->endTime = grainDurSamps * increment + grain->currTime;
	//std::cout<<"sending grain with start time : "<< grain->currTime << " first sample : " << buffer->Get(grain->currTime) << "\n";
}

void STGRAN2::resetgraincounter()
{
	newGrainCounter = (int)round(SR * prob(grainRateVarLow, grainRateVarMid, grainRateVarHigh, grainRateVarTight));
}

// update pfields
void STGRAN2::doupdate()
{
	double p[22];
	update(p, 22); // this could be fixed to only update necessary p-fields
	amp =(float) p[3];

	grainDurLow = (double)p[8];
	grainDurMid = (double)p[9]; if (grainDurMid < grainDurLow) grainDurMid = grainDurLow;
	grainDurHigh = (double)p[10]; if (grainDurHigh < grainDurMid) grainDurHigh = grainDurMid;
	grainDurTight = (double)p[11];


	grainRateVarLow = (double)p[4];
	grainRateVarMid = (double)p[5]; if (grainRateVarMid < grainRateVarLow) grainRateVarMid = grainRateVarLow;
	grainRateVarHigh = (double)p[6]; if (grainRateVarHigh < grainRateVarMid) grainRateVarHigh = grainRateVarMid;
	grainRateVarTight = (double)p[7];

	transLow = octpch((double)p[12]);
	transMid = octpch((double)p[13]); if (transMid < transLow) transMid = transLow;
	transHigh = octpch((double)p[14]); if (transHigh < transMid) transHigh = transMid;
	transTight = octpch((double)p[15]);


	panLow = (double)p[16];
	panMid = (double)p[17]; if (panMid < panLow) panMid = panLow;
	panHigh = (double)p[18]; if (panHigh < panMid) panHigh = panMid;
	panTight = (double)p[19];

	if (_nargs > 21)
	{
		int bufferSize = (int) floor(SR * p[21]);
		
		if (bufferSize > MAXBUFFER)
		{
			rtcmix_advise("STGRAN2", "Buffer size capped at 10 seconds at 44.1k sample rate");
			bufferSize = MAXBUFFER;
		}
		buffer->SetSize(bufferSize);
	}

}

int STGRAN2::run()
{	
	//std::cout<<"new control block"<<"\n";
	float out[2];
	int samps = framesToRun() * inputChannels();

	rtgetin(in, this, samps);
	//int grainsCurrUsed = 0;
	for (int i = 0; i < samps; i += inputChannels()) {
		buffer->Append(in[i + inchan]); // currently only takes the left input
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
				if ((*currGrain).currTime > currGrain->endTime)
				{
					currGrain->isplaying = false;
				}
				else
				{
					// at some point, make your own interpolation
					float grainAmp = oscil(1, currGrain->ampSampInc, grainEnv, grainEnvLen, &((*currGrain).ampPhase));
					float grainOut = grainAmp * buffer->Get(currGrain->currTime);
					currGrain->currTime += currGrain->waveSampInc;
					// std::cout<<" outputing grain " << grainAmp << "\n";
					out[0] += grainOut * currGrain->panL;
					out[1] += grainOut * currGrain->panR;
				}
			}
			// this is not an else statement so a grain can be potentially stopped and restarted on the same frame

			if ((newGrainCounter <= 0) && !currGrain->isplaying)
			{
				resetgraincounter();
				if (newGrainCounter > 0) // we don't allow two grains to be created on the same frame
					{resetgrain(currGrain);
					
					}
				else
					{newGrainCounter = 1;
					}

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
	return framesToRun();
}


Instrument *makeSTGRAN2()
{
	STGRAN2 *inst = new STGRAN2();
	inst->set_bus_config("STGRAN2");

	return inst;
}


#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("STGRAN2", makeSTGRAN2);
}
#endif

