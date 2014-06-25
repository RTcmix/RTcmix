#include "metaflute.h"

class LSFLUTE : public Instrument {
	int length1,length2;
	float amp,namp,dampcoef,oldsig;
	double *amparr, *oamparr;
	float amptabs[2], oamptabs[2];
	float ampmult, spread;
	float aamp, oamp;
	int skip, branch;

public:
	LSFLUTE();
	virtual int init(double*, int);
	virtual int run();
};
