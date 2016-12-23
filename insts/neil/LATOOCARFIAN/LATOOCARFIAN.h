#include <Instrument.h>

class LATOOCARFIAN : public Instrument {

public:
	LATOOCARFIAN();
	virtual ~LATOOCARFIAN();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int nargs, branch;
	float amp, pan, x, prevx, y, param1, param2, param3, param4;
};

