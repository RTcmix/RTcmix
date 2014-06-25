#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include <rt.h>
#include <rtdefs.h>
#include "FADE_HOLD.h"

Bool start_fade = NO;
Bool fade_started = NO;
Bool stop_hold;

FADE_HOLD::FADE_HOLD() : Instrument()
{

}

FADE_HOLD::~FADE_HOLD()
{

}

int FADE_HOLD::init(double p[], int n_args)
{
	float start,dur;	

	start = p[0];
	dur = 0.001;
	
	if (rtsetoutput(start, dur, this) != 0)
		return DONT_SCHEDULE;
	
	return(0);
}

int FADE_HOLD::run()
{
	
	start_fade = YES;
	stop_hold = NO;
	return(0);
	
}

Instrument*
makeFADE_HOLD()
{
	FADE_HOLD *inst;
	inst = new FADE_HOLD();
	inst->set_bus_config("FADE_HOLD");
	return inst;
}


