/* JGNoise Generator Class, by Perry R. Cook, 1995-96
   Returns pseudo-random numbers in the range (-1.0, 1.0).
   (Seed arg and method added by JGG.)
*/
#include "JGNoise.h" 
// BGGx ww
//#include <sys/time.h>
#include <time.h> // MS version

// BGGx ww
#define __OS_Win_

#if defined(__OS_Win_)                              // for Windows95 or NT
   #define ONE_OVER_RANDLIMIT 0.00006103516
#else                                               // for Linux and SGI
   #define ONE_OVER_RANDLIMIT 0.00000000093132258
#endif


/* <aSeed> can be any integer between 0 and UINT_MAX. If it's 0, or if
   the argument is not given, then seed taken from microsecond counter.
*/
JGNoise :: JGNoise(unsigned int aSeed)
{
   this->seed(aSeed);

   lastOutput = 0.0;
}


JGNoise :: ~JGNoise()
{
}


/* <aSeed> can be any integer between 0 and UINT_MAX. If it's 0, or if
   the argument is not given, then seed taken from microsecond counter.
*/
void JGNoise :: seed(unsigned int aSeed = 0)
{
   if (aSeed == 0) {
	// BGGx ww
	// struct timeval tv;
	// gettimeofday(&tv, NULL);
	// aSeed = (unsigned int)tv.tv_usec;
        time_t ltime;
        time(&ltime);
	aSeed = ltime;
   }
   // BGGx ww -- use srand() instead of srandom()
   //   srandom(aSeed);
   srand(aSeed);
}


double JGNoise :: tick()
{
#if defined(__OS_Win_)
   lastOutput = (double) (rand() - 16383);
#else
   lastOutput = (double) random() - 1073741823.0;
#endif

   lastOutput *= ONE_OVER_RANDLIMIT;
   return lastOutput;
}


double JGNoise :: lastOut()
{
   return lastOutput;
}


