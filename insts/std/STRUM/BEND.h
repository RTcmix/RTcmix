#include "strums.h"

class BEND : public Instrument {
	float freq0,freq1,diff;
	float tf0,tfN;
	float *glissf,tags[2];
	float *amptable, amptabs[2];
	float spread,aamp;
	strumq *strumq1;
	int reset,branch;

public:
	BEND();
	int init(double*, int);
	int run();
	};
