#include "strums.h"

class FRET : public Instrument {
	float freq,tf0,tfN;
	float spread, aamp, amptabs[2];
	double *amptable;
	strumq *strumq1;
	int firsttime,branch,skip;

public:
	FRET();
	int init(double*, int);
	int run();
	};
