#include "gverbdsp.h"
#include "gverbdefs.h"

class GVERB : public Instrument {
	ty_gverb realp;
	ty_gverb *p;
	float *in;

	float amp;
	int branch;
	int inputframes;
	int inputchan;

	void doupdate();

public:
	GVERB();
	virtual ~GVERB();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};
