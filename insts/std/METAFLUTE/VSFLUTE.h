#include "metaflute.h"

class VSFLUTE : public Instrument {
	int dl1[2],dl2[2];
	float l1span,l2span,l1base,l2base;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	float *amparr,amptabs[2];
	int lenamp;
	float *oamparr,oamptabs[2];
	int olenamp;
	float *pcurve1,si1lo,si1hi;
	int psize1;
	float *pcurve2,si2lo,si2hi;
	int psize2;
	float ampmult, spread;
	int skip;
	float phs1,phs2;

public:
	VSFLUTE();
	int init(float*, short);
	int run();
	};
