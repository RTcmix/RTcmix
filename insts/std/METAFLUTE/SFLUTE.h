#include "metaflute.h"

class SFLUTE : public Instrument {
	int dl1[3],dl2[3];
	float del1[DELSIZE],del2[DELSIZE];
	int length1,length2;
	float amp,namp,dampcoef,oldsig;
	double *amparr, *oamparr;
	float amptabs[2], oamptabs[2];
	float ampmult, spread;
	float aamp, oamp;
	int skip, branch;

public:
	SFLUTE();
	virtual int init(double*, int);
	virtual int run();
};
