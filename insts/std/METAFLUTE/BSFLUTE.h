#include "metaflute.h"

class BSFLUTE : public Instrument {
	int dl1[2],dl2[2];
	float l1span,l2span,l1base,l2base;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	double *amparr, *oamparr, *pcurve1, *pcurve2;
	float amptabs[2], oamptabs[2], ptabs1[2], ptabs2[2];
	float ampmult, spread;
	int skip;

public:
	BSFLUTE();
	int init(double*, int);
	int run();
	};
