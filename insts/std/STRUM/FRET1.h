#include "strums.h"
#include "delayq.h"

class FRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	float spread,amp,aamp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d, amptabs[2];
	double *amptable;
	int firsttime,branch,skip;

public:
	FRET1();
	int init(double*, int);
	int run();
	};
