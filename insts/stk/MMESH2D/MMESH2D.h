#include <Instrument.h>      /* the base class for this instrument */

class Ooscili;
class Mesh2D;
class Odcblock;

class MMESH2D : public Instrument {
	int nargs, branch;
	float amp, pctleft;
	double *amptable;
	Ooscili *theEnv;
	Mesh2D *theMesh;
	Odcblock *dcblocker;

public:
	MMESH2D();
	virtual ~MMESH2D();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kPan = 1 << 9
};
