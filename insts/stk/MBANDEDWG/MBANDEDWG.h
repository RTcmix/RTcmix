#include <Ougens.h>

class MBANDEDWG : public Instrument {
	float   amp, pctleft;
	double   amparray[2];
	double   velarray[2];
	Ooscili *theEnv;
	Ooscili *theVeloc;
	BandedWG *theBar;

public:
	MBANDEDWG();
	virtual ~MBANDEDWG();
	virtual int init(double *, int);
	virtual int run();
};

