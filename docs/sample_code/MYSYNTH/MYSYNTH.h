#include <Instrument.h>		  // the base class for this instrument

class MYSYNTH : public Instrument {

public:
	MYSYNTH();
	virtual ~MYSYNTH();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _amp, _pan;
};

