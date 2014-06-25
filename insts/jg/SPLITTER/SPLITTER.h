#include <Instrument.h>      // the base class for this instrument

class SPLITTER : public Instrument {

public:
	SPLITTER();
	virtual ~SPLITTER();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _inchan, _branch, _nargs;
	float _amp;
	float *_in;
	float *_amps;
};

