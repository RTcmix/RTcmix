#include "metaflute.h"

class SFLUTE : public Instrument {
	int dl1[2],dl2[2];
	float del1[DELSIZE],del2[DELSIZE];
	int length1,length2;
	float amp,namp,dampcoef,oldsig;
	float *amparr,amptabs[2];
	int lenamp;
	float *oamparr,oamptabs[2];
	int olenamp;
	float ampmult, spread;
	int skip;

public:
	SFLUTE();
	int init(float*, int);
	int run();
	};
