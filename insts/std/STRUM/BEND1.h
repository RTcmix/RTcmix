#include "strums.h"
#include "delayq.h"

class BEND1 : public Instrument {
	float freq0,diff;
	float tf0,tfN;
	float *glissf,tags[2];
	float spread,amp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	int reset;

public:
	BEND1();
	int init(float*, int);
	int run();
	};
