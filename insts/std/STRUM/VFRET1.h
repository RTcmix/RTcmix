#include "strums.h"
#include "delayq.h"

class VFRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	int vlen, branch1, branch2;
	float *vloc;
	float vsibot,vsidiff,vsi,vphase;
	float vdepth;
	float *eloc,tab[2];
	float spread,amp,aamp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	float *amptable, amptabs[2];
	int reset;
	int firsttime;

public:
	VFRET1();
	int init(double*, int);
	int run();
	};
