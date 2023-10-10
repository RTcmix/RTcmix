#include <Instrument.h>      // the base class for this instrument

typedef double (*mathfunctionpointer)(double);

class DSPMATH : public Instrument {

public:
    DSPMATH();
	virtual ~DSPMATH();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	float *_in;
	int _branch;
	float _amp, _drive;
    mathfunctionpointer _mathfun;
};

