#include "metaflute.h"

class BSFLUTE : public Instrument {
	int dl1[3],dl2[3];
	float l1span,l2span,l1base,l2base;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	float aamp, oamp;
	float length1, length2;
	double *amparr, *oamparr, *pcurve1, *pcurve2;
	float amptabs[2], oamptabs[2], ptabs1[2], ptabs2[2];
	float ampmult, spread;
	int skip, branch;

public:
	BSFLUTE();
	virtual int init(double*, int);
	virtual int run();
};
