#include "metaflute.h"

class LSFLUTE : public Instrument {
	int length1,length2;
	float amp,namp,dampcoef,oldsig;
	float *amparr,amptabs[2];
	float *oamparr,oamptabs[2];
	float ampmult, spread;
	int skip;

public:
	LSFLUTE();
	int init(float*, int);
	int run();
	};
