// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "allpass.hpp"

fv_allpass::fv_allpass() : antidenorm(1e-18f), bufidx(0)
{
}

void fv_allpass::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

void fv_allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void fv_allpass::setfeedback(float val) 
{
	feedback = val;
}

float fv_allpass::getfeedback() 
{
	return feedback;
}

//ends
