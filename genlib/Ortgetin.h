/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ORTGETIN_H_
#define _ORTGETIN_H_ 1

class Instrument;

class Ortgetin
{
	Instrument *theInst;
	int chns, rsamps, chptr;
	float *in;

public:
	Ortgetin(Instrument *);
	int next(float *);
};

#endif // _ORTGETIN_H_
