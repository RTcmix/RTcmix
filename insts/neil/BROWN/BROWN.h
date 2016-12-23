#include <Instrument.h>		  // the base class for this instrument

class BROWN : public Instrument {

public:
	BROWN();
	virtual ~BROWN();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _brown, _amp, _pan;
};

