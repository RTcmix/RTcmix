#include "strums.h"
#include "delayq.h"

class FRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	float spread,amp,aamp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	float *amptable, amptabs[2];
	int firsttime,branch,skip;

public:
	FRET1();
	int init(float*, int);
	int run();
	};
