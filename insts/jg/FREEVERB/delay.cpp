// Delay line implementation
// written by JGG, 3 Feb 2001 - based on allpass.cpp

#include "delay.hpp"

fv_delay::fv_delay()
{
	bufidx = 0;
}

void fv_delay::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

void fv_delay::mute()
{
	for (int i = 0; i < bufsize; i++)
		buffer[i] = 0;
}

void fv_delay::setdelaysamps(int val) 
{
	if (val >= 0 && val <= bufsize)
		delaysamps = val;
}

int fv_delay::getdelaysamps() 
{
	return delaysamps;
}

