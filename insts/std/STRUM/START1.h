#include "strums.h"
#include "delayq.h"

class START1 : public Instrument {
	float spread, amp;
	strumq *strumq1;
	delayq *dq;
	float dgain, fbgain;
	float cleanlevel, distlevel;
	float d;
	int deleteflag;

public:
	START1();
	virtual ~START1();
	int init(float*, short);
	int run();
	};
