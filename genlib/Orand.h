/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _ORAND_H_
#define _ORAND_H_ 1

class Orand
{
	long rand_x;

public:
	Orand();
	Orand(int seed);

	void seed(int seed);

	// seed with the time of day
	void timeseed();

	// returns between 0.0 and 1.0 -- based on brrand() in randfuncs.c
	float random();

	// returns between -1 and +1 -- based on rrand()
	float rand();

	// returns between min and max
	float range(float min, float max);
};

#endif // _ORAND_H_
