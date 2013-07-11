/* MAXMESSAGE -- set to work with max/msp check_vals().

	this one returns the number of RTcmix values to return
	back to max/msp to be sent out as a float or as a list
	of floats.  Values are set via the p-fields of this
	instrument.  It then sets an extern "C" int 'vals_ready'
	to the number of vals (0 means no vals pending) at the
	for check_vals at the end of Minc/main.C

	p0 = time to send them vals
	p1-n = the vals

	BGG 1/2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "MAXMESSAGE.h"
#include <rt.h>
#include <rtdefs.h>


MAXMESSAGE::MAXMESSAGE() : Instrument()
{
}

MAXMESSAGE::~MAXMESSAGE()
{
}


int MAXMESSAGE::init(double p[], int n_args)
{
	int i;

	if (rtsetoutput(p[0], 0, this) == -1)
		return DONT_SCHEDULE;

	for (i = 0; i < n_args-1; i++) thevals[i] = p[i+1];
	nvals = n_args-1;

	return nSamps();
}

extern "C" int vals_ready;
extern "C" float maxmsp_vals[];

int MAXMESSAGE::run()
{
	int i;

	for (i = 0; i < nvals; i++) maxmsp_vals[i] = thevals[i];
	vals_ready = nvals;

	return(1);
}



Instrument*
makeMAXMESSAGE()
{
	MAXMESSAGE *inst;

	inst = new MAXMESSAGE();
	inst->set_bus_config("MAXMESSAGE");

	return inst;
}
/*
void
rtprofile()
{
	RT_INTRO("MAXMESSAGE",makeMAXMESSAGE);
}
*/

