#include "strums.h"

class FRET : public Instrument {
	float freq,tf0,tfN;
	float spread,aamp;
	float *amptable, amptabs[2];
	strumq *strumq1;
	int firsttime,branch,skip;

public:
	FRET();
	int init(double*, int);
	int run();
	};
