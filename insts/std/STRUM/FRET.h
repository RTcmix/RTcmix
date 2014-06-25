#include "strums.h"

class FRET : public Instrument {
	float freq,tf0,tfN;
	float spread, aamp, amptabs[2];
	double *amptable;
	StrumQueue *strumq1;
	int branch,skip;

public:
	FRET();
	virtual ~FRET();
	virtual int init(double*, int);
	virtual int configure();
	virtual int run();
};
