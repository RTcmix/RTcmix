#include "strums.h"

class BEND : public Instrument {
	float freq0,freq1,diff;
	float tf0,tfN;
	float tags[2], amptabs[2];
	double *amptable, *glissf;
	float spread,aamp;
	StrumQueue *strumq1;
	int reset,branch;

public:
	BEND();
	virtual ~BEND();
	virtual int init(double*, int);
	virtual int run();
};
