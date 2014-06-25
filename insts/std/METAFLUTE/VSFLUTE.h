#include "metaflute.h"

class VSFLUTE : public Instrument {
	int dl1[3],dl2[3];
	float l1span,l2span,l1base,l2base;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	double *amparr, *oamparr, *pcurve1, *pcurve2;
	float amptabs[2], oamptabs[2];
	float si1lo,si1hi;
	int psize1;
	float si2lo,si2hi;
	int psize2;
	float ampmult, spread;
	int skip, branch;
	float phs1,phs2;
	float aamp, oamp;
	float si1, si2;

public:
	VSFLUTE();
	virtual int init(double*, int);
	virtual int run();
};
