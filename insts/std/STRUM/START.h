#include "strums.h"

class START : public Instrument {
	float spread;
	float *amptable, amptabs[2];
	strumq *strumq1;
	int deleteflag, skip;

public:
	START();
	virtual ~START();
	int init(float*, int);
	int run();
	};
