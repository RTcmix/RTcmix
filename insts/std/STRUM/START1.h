#include "strums.h"
#include "delayq.h"

class START1 : public Instrument {
	float spread, amp, aamp;
	StrumQueue *strumq1;
	DelayQueue *dq;
	float dgain, fbgain;
	float cleanlevel, distlevel;
	float d, amptabs[2];
	double *amptable;
	int deleteflag, branch, skip;

public:
	START1();
	virtual ~START1();
	virtual int init(double*, int);
	virtual int run();
};
