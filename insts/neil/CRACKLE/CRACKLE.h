#include <Instrument.h>

class CRACKLE : public Instrument {

public:
	CRACKLE();
	virtual ~CRACKLE();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int nargs, branch;
	float amp, pan, param, x0, x1, x2, x3;
};

