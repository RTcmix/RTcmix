//  pRTcmix.C -- Perl addition to RTcmix

/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "pRTcmix.h"
//#include <sys/time.h>

extern "C" {
#include "rtcmix_parse.h"
}

//  Constructor with default SR, NCHANS, and RTBUFSAMPS
PRTcmix::PRTcmix() : RTcmix()
{
}

//  Constructor with settable SR, NCHANS; default RTBUFSAMPS
PRTcmix::PRTcmix(float tsr, int tnchans) : RTcmix(tsr, tnchans)
{
}

//  Constructor with settable SR, NCHANS, and RTBUFSAMPS
PRTcmix::PRTcmix(float tsr, int tnchans, int bsize) : RTcmix(tsr, tnchans, bsize)
{
}

void PRTcmix::perlparse(char *inBuf)
{
// 	double buftime,sec,usec;
// 	struct timeval tv;
// 	struct timezone tz;
// 
// 	buftime = (double)RTBUFSAMPS/SR;
// 	
// 	gettimeofday(&tv, &tz);
// 	sec = (double)tv.tv_sec;
// 	usec = (double)tv.tv_usec;
// 	pthread_mutex_lock(&schedtime_lock);
// 	schedtime = (((sec * 1e6) + usec) - baseTime) * 1e-6;
// 	schedtime += ((double)elapsed/(double)SR);
// 	schedtime += buftime*5;
// 	pthread_mutex_unlock(&schedtime_lock);
	perl_parse_buf(inBuf);
}
