#include "strums.h"
#include "delayq.h"

class VSTART1 : public Instrument {
	float freq,tf0,tfN;
	int vlen, branch1, branch2;
	double *eloc, *vloc, *amptable;
	float vsibot,vsidiff,vsi,vphase;
	float vdepth, tab[2];
	float spread, amp, aamp;
	StrumQueue *strumq1;
	DelayQueue *dq;
	float dgain, fbgain;
	float cleanlevel, distlevel;
	int reset;
	float d, amptabs[2];
	int deleteflag;

public:
	VSTART1();
	virtual ~VSTART1();
	virtual int init(double*, int);
	virtual int run();
};
