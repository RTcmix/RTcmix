#include "strums.h"

class START : public Instrument {
	float spread, amptabs[2];
	double *amptable;
	strumq *strumq1;
	int deleteflag, skip;

public:
	START();
	virtual ~START();
	int init(double*, int);
	int run();
	};
