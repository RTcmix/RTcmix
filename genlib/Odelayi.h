/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ODELAYI_H_
#define _ODELAYI_H_ 1

#include "Odelay.h"

// Interpolating delay class, offering two different ways of storing and
// retrieving values from the delay line.  (See API 1 and API 2 below.) 
// It's best not to mix the two ways while working with an Odelayi object.
// Based on cmix delset/dliget and STK DLineL.              -JGG, 7/9/04


class Odelayi : public Odelay
{
public:
	Odelayi(long defaultLength);
	virtual ~Odelayi();

	// Get sample from delay line that is <lagsamps> samples behind the most
	// recent sample to enter the delay line.  If <lagsamps> is longer than
	// length of delay line, it wraps around, so check <lagsamps> first.

	virtual float getsamp(double lagsamps);

	// --------------------------------------------------------------------------
	// API 2: setdelay / next
	//
	// Set the delay in samples by calling setdelay(), then call next() to store
	// a new value into delay line and retreive the oldest value.  Does not let
	// you have more than one delay tap.  Based on STK DLineL implementation.

	virtual void setdelay(double lagsamps);
	virtual float next(float input);

	// The current delay in samples.
	virtual float delay() const;

private:
	double _frac;
};

#endif // _ODELAYI_H_
