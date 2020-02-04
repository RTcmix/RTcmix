#include <Instrument.h>      // the base class for this instrument

class TONE : public Instrument {

public:
	TONE();
	virtual ~TONE();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	float *_in;
	int _nargs, _branch;
	float _amp, _cf;
	double toneData[4][3];
};

