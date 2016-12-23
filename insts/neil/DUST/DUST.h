#include <Instrument.h>		  // the base class for this instrument

class DUST : public Instrument {

public:
	DUST();
	virtual ~DUST();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _amp, _pan, _density, _range;
	Orand *_dice;
};

