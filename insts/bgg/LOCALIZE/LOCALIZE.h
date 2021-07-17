#include <Instrument.h>      // the base class for this instrument
#include <Ougens.h>
#include <math.h>

class LOCALIZE : public Instrument {

public:
	LOCALIZE();
	virtual ~LOCALIZE();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

	void doupdate();

private:
	int _branch;

	Odelayi *theDelayL;
	Odelayi *theDelayR;
	Ortgetin *theIn;
	Oonepole *headfilt;

	float _amp;
	float dist;
	float theta;
	float headdelay;

	float _ampL, _ampR;
	float _delayL, _delayR;
	int _dofilt;
	int _doampdist;
	float _totlindist;
	float _mindistamp;
	int _inputchan;
};

