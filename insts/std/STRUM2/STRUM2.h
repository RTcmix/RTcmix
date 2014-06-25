#include <Instrument.h>

class Ostrum;

class STRUM2 : public Instrument {

public:
	STRUM2();
	virtual ~STRUM2();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _amp, _rawfreq, _pan;
	Ostrum *_strum;
};

