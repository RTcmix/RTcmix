// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "comb.hpp"

#ifdef ANTI_DENORM
fv_comb::fv_comb() : filterstore(0), antidenorm(1e-18f), bufidx(0)
#else
fv_comb::fv_comb() : filterstore(0), bufidx(0)
#endif
{
}

void fv_comb::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

void fv_comb::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

void fv_comb::setdamp(float val) 
{
	damp1 = val; 
	damp2 = 1-val;
}

float fv_comb::getdamp() 
{
	return damp1;
}

void fv_comb::setfeedback(float val) 
{
	feedback = val;
}

float fv_comb::getfeedback() 
{
	return feedback;
}

// ends
