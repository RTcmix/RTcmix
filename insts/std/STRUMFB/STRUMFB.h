#include <Instrument.h>

class Odelayi;
class Odistort;
class Ostrum;

class STRUMFB : public Instrument {

public:
	STRUMFB();
	virtual ~STRUMFB();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _branch;
	float _amp, _rawfreq, _rawfbfreq, _delsamps, _decaytime, _nyqdecaytime, _pan;
	float _last, _distgain, _fbgain, _cleanlevel, _distlevel;
	Odelayi *_delay;
	Odistort *_distort;
	Ostrum *_strum;
};

