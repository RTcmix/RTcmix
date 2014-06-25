#include <Instrument.h>      // the base class for this instrument

class Odcblock;

class DCBLOCK : public Instrument {

public:
	DCBLOCK();
	virtual ~DCBLOCK();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _branch, _chans;
	float _amp;
	float *_in;
	Odcblock **_blocker;
};

