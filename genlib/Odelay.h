/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ODELAY_H_
#define _ODELAY_H_ 1

// Delay class, offering two different ways of storing and
// retrieving values from the delay line.  (See API 1 and API 2 below.) 
// It's best not to mix the two ways while working with an Odelay object.
// Based on cmix delset/dliget and STK DLineL.              -JGG, 7/9/04


class Odelay
{
public:
	Odelay(long defaultLength);
	virtual ~Odelay();
	void clear();

	// --------------------------------------------------------------------------
	// API 1: putsamp / getsamp
	//
	// Put sample into delay line using putsamp().  Retreive sample from any
	// point in delay line, specified by floating-point number of samples, using
	// getsamp().  Unlike API 2 (below), getsamp does not affect the delay line
	// input pointer, so you can call getsamp multiple times for every call to
	// putsamp, letting you implement multiple delay taps.  Based on classic
	// cmix genlib delset/dliget, except it takes the number of samples of delay
	// rather than a delay time in seconds.
	//
	// Note that this API does not maintain the correct values for _outpoint and
	// _frac across calls to getsamp, so if you want to use API 2 after API 1 for
	// the same Odelay object, then be sure to call setdelay before using API 2.

	// Put sample into delay line, and advance input pointer.  Use getsamp() to
	// retrieve samples from delay line at varying delays from this input
	// pointer.

	void putsamp(float samp);

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

	float last() { return _lastout; }
	
protected:
	int	  resize(long newLen);

protected:
	float *_dline;
	long _len;
	long _inpoint;
	long _outpoint;
	float _lastout;
};

#endif // _ODELAYI_H_
