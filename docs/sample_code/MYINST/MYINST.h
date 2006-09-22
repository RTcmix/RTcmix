#include <Instrument.h>      // the base class for this instrument

class MYINST : public Instrument {

public:
	MYINST();
	virtual ~MYINST();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	float *_in;
	int _nargs, _inchan, _branch;
	float _amp, _pan;
};

