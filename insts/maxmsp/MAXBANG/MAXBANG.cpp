/* MAXBANG -- set to work with max/msp check_bang().

	all this does is set the extern "C" int 'bang_ready' to 1,
	max/msp will check this and send out a bang (and reset it
	to 0 in check_bang, at the end of Minc/main.C

	p0 = time to generate the bang

	BGG 1/2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "MAXBANG.h"
#include <rt.h>
#include <rtdefs.h>


MAXBANG::MAXBANG() : Instrument()
{
}

MAXBANG::~MAXBANG()
{
}


int MAXBANG::init(double p[], int n_args)
{
	if (rtsetoutput(p[0], 0, this) == -1)
		return DONT_SCHEDULE;

	return nSamps();
}

extern "C" int bang_ready;

int MAXBANG::run()
{
	bang_ready = 1;
	return(1);
}



Instrument*
makeMAXBANG()
{
	MAXBANG *inst;

	inst = new MAXBANG();
	inst->set_bus_config("MAXBANG");

	return inst;
}
/*
void
rtprofile()
{
	RT_INTRO("MAXBANG",makeMAXBANG);
}
*/

