#include "strums.h"
#include "delayq.h"

class VSTART1 : public Instrument {
	float freq,tf0,tfN;
	int vlen;
	float *vloc;
	float vsibot,vsidiff,vsi,vphase;
	float vdepth;
	float *eloc,tab[2];
	float spread, amp;
	strumq *strumq1;
	delayq *dq;
	float dgain, fbgain;
	float cleanlevel, distlevel;
	int resetval;
	float d;
	int deleteflag;

public:
	VSTART1();
	virtual ~VSTART1();
	int init(float*, short);
	int run();
	};
