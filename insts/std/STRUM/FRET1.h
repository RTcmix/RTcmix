#include "strums.h"
#include "delayq.h"

class FRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	float spread,amp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	int firsttime;

public:
	FRET1();
	int init(float*, int);
	int run();
	};
