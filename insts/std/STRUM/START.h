#include "strums.h"

class START : public Instrument {
	float spread, aamp, amptabs[2];
	double *amptable;
	StrumQueue *strumq1;
	int deleteflag, skip, branch;

public:
	START();
	virtual ~START();
	virtual int init(double*, int);
	virtual int run();
};
