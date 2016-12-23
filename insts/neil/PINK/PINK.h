#include <Instrument.h>		  // the base class for this instrument

class PINK : public Instrument {

public:
	PINK();
	virtual ~PINK();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	double _b0, _b1, _b2, _b3, _b4, _b5, _b6;
	float _amp, _pan;
};

