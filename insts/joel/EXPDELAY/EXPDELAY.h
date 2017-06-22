#include <Instrument.h>      // the base class for this instrument

class EXPDELAY : public Instrument {

public:
	EXPDELAY();
	virtual ~EXPDELAY();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();
	float *_delay_buf;
	float *_in;
	int _nargs, _inchan, _branch;
	float _amp, _pan;
	float _max_delay_time;
	int _delay_reps;
	float _delay_time;
	float _amp_curve, _decay_curve;
	int _buffer_pos;
};
