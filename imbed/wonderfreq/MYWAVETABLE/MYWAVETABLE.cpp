#include <Instrument.h>
#include "MYWAVETABLE.h"
#include <rt.h>
#include <rtdefs.h>

MYWAVETABLE::MYWAVETABLE() : Instrument(), _updateCounter(0)
{
}

MYWAVETABLE::~MYWAVETABLE()
{
}

int MYWAVETABLE::init(double p[], int n_args)
{
// p0 = start; p1 = dur; p2 = amplitude; p3 = frequency; p4 = stereo spread;

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	theEnv = new Ooscili(SR, 1.0/p[1], 1); // assumes makegen 1 for amp env

	_amp = p[2];

	theOscil = new Ooscili(SR, _pitch = p[3], 2); // assumes makegen 2 for waveform

	_spread = p[4];
	
	return nSamps();
}

int MYWAVETABLE::run()
{
	float out[2];
	
	for (int i = 0; i < framesToRun(); i++) {
		// Here is where we call Instrument::update() at whatever rate
		// we desire.  We check for pitch diffs to avoid extra work.
		if (--_updateCounter <= 0) {
			double p[5];
			update(p, 5);
			_amp = p[2];
			if (p[3] != _pitch) {
				_pitch = p[3];
				theOscil->setfreq(_pitch);
			}
			_spread = p[4];
			_updateCounter = getSkip();
		}
		out[0] = theOscil->next() * theEnv->next() * _amp;
		
		if (outputChannels() == 2) // split stereo files between channels
		{
			out[1] = (1.0 - _spread) * out[0];
			out[0] *= _spread;
		}
	  
		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument*
makeMYWAVETABLE()
{
	MYWAVETABLE *inst;
	inst = new MYWAVETABLE();
	inst->set_bus_config("MYWAVETABLE");
	return inst;
}

void
rtprofile()
{
	RT_INTRO("MYWAVETABLE",makeMYWAVETABLE);
}

