#include <Instrument.h>

class HENON : public Instrument {

public:
	HENON();
	virtual ~HENON();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();
	void updateparams();

	int nargs, branch, branch2, cr;
	float amp, pan, a, b, x, y, z;
};

