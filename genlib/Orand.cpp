#include <ugens.h>
#include "../H/Ougens.h"
#include <sys/time.h>

Orand::Orand()
{
	rand_x = 1;
}

Orand::Orand(int x)
{
	rand_x = x;
}

void Orand::seed(int x)
{
	rand_x = x;
}

void Orand::timeseed()
	// seed with the time of day
{
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv,&tz);
	seed(tv.tv_usec);
}

float Orand::random()
	// between 0.0 and 1.0 -- based on brrand() in randfuncs.c
{
	int i = ((rand_x = rand_x*1103515245 + 12345)>>16) & 077777;
	return((float)i/32768.0);
}

float Orand::rand()
	// between -1 and +1 -- based on rrand()
{
	int i = ((rand_x = rand_x*1103515245 + 12345)>>16) & 077777;
	return((float)i/16384. - 1.);
}

float Orand::range(float lo, float hi)
	// between lo and hi (obviously)
{
	return(random() * (hi-lo) + lo);
}
