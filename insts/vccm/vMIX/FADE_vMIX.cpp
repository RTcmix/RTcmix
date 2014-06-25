#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>
#include "FADE_vMIX.h"

Bool start_fade = NO;
// Bool fade_started = NO;
// Bool stop_hold;

FADE_vMIX::FADE_vMIX() : Instrument()
{

}

FADE_vMIX::~FADE_vMIX()
{

}

int FADE_vMIX::init(double p[], int n_args)
{
	float start;

	start = p[0];
	dur = 0.001;
	
	if (rtsetoutput(start, dur, this) != 0)
		return DONT_SCHEDULE;
	
	return(0);
}

int FADE_vMIX::run()
{
	start_fade = YES;
	//	stop_hold = NO;
	return(0);
}

Instrument*
makeFADE_vMIX()
{
	FADE_vMIX *inst;
	inst = new FADE_vMIX();
	inst->set_bus_config("FADE_vMIX");
	return inst;
}


