#include "metaflute.h"

class BSFLUTE : public Instrument {
	int dl1[2],dl2[2];
	float l1span,l2span,l1base,l2base;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	float *amparr,amptabs[2];
	float *oamparr,oamptabs[2];
	float *pcurve1,ptabs1[2];
	float *pcurve2,ptabs2[2];
	float ampmult, spread;
	int skip;

public:
	BSFLUTE();
	int init(float*, int);
	int run();
	};
