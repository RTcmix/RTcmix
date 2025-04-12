// RVB.h--	Post-processing reverb.  Takes dry signal from normal input and
// 			summed first and second generation reflections from a global array

#ifndef _RTCMIX_RVB_H_
#define _RTCMIX_RVB_H_

#include <RVBBASE.h>

#define RVB_CHANS 2
#define RVB_DELCOUNT 6

class RVB : public RVBBASE<RVB_CHANS, RVB_DELCOUNT> {
public:
	RVB();
	virtual ~RVB();
protected:
    virtual void get_lengths(long);
    virtual void set_random();
};

#endif	// _RTCMIX_RVB_H_

