#include <Instrument.h>      // the base class for this instrument

class LOOP : public Instrument {

public:
	LOOP();
	virtual ~LOOP();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	int doupdate();
	int calculateLoop(double *pArray);
	
	float *_in;
	bool _usesPan;
	int _inchan, _branch, _inOffset, _inLoc;
	float _amp, _pan, _loopStart, _loopEnd;
	double _position, _incr, _lastInPosition;
};

