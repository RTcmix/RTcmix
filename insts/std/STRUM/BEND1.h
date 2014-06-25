#include "strums.h"
#include "delayq.h"

class BEND1 : public Instrument {
	float freq0,diff;
	float tf0,tfN;
	double *glissf, *amptable;
	float tags[2], amptabs[2];
	float spread,amp,aamp;
	StrumQueue *strumq1;
	DelayQueue *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	int reset,branch;

public:
	BEND1();
	virtual ~BEND1();
	virtual int init(double*, int);
	virtual int run();
};
