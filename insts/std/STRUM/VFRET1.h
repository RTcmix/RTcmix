#include "strums.h"
#include "delayq.h"

class VFRET1 : public Instrument {
	float freq,tf0,tfN,fbpitch;
	int vlen;
	float *vloc;
	float vsibot,vsidiff,vsi,vphase;
	float vdepth;
	float *eloc,tab[2];
	float spread,amp;
	strumq *strumq1;
	delayq *dq;
	float dgain,fbgain;
	float cleanlevel,distlevel;
	float d;
	int reset;
	int firsttime;

public:
	VFRET1();
	int init(float*, int);
	int run();
	};
