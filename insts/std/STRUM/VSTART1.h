#include "strums.h"
#include "delayq.h"

class VSTART1 : public Instrument {
	float freq,tf0,tfN;
	int vlen, branch1, branch2;
	float *vloc;
	float vsibot,vsidiff,vsi,vphase;
	float vdepth;
	float *eloc,tab[2];
	float spread, amp, aamp;
	strumq *strumq1;
	delayq *dq;
	float dgain, fbgain;
	float cleanlevel, distlevel;
	int reset;
	float d;
	float *amptable, amptabs[2];
	int deleteflag;

public:
	VSTART1();
	virtual ~VSTART1();
	int init(double*, int);
	int run();
	};
