#include "strums.h"
#include "delayq.h"

class FRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	float spread,amp,aamp;
	StrumQueue *strumq1;
	DelayQueue *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d, amptabs[2];
	double *amptable;
	int branch,skip;

public:
	FRET1();
	virtual ~FRET1();
	virtual int init(double*, int);
	virtual int configure();
	virtual int run();
};
