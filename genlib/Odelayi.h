/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ODELAYI_H_
#define _ODELAYI_H_ 1

#include "Odelay.h"

// Interpolating delay class, offering two different ways of storing and
// retrieving values from the delay line.  (See API 1 and API 2 in Odelay.h.) 

class Odelayi : public Odelay
{
public:
	Odelayi(long defaultLength);
	virtual ~Odelayi();

	// See Odelay.h (base class) for description of these functions.
	virtual float getsamp(double lagsamps);
	virtual void setdelay(double lagsamps);
	virtual float next(float input);
	virtual float delay() const;

private:
	double _frac;
};

#endif // _ODELAYI_H_
