/* PFSCHED - 'vehicle' instrument to allow schedule PField values

	PFSCHED takes a PField as an input and will read from it
	at a scheduled time for a scheduled duration.  These values
	are then written to a 'pfield bus' specified in a makeconnection()
	scorefile cammand.  The PField handle from that makeconnection()
	can then be used to feed scheduled PField values into an
	executing instrument

	p0 = output start time
	p1 = duration
	p2 = 'pfield bus' to use
	p3 = input PField variable

	BGG, 11/2009
*/

#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include "PFSCHED.h"
#include <rt.h>
#include <rtdefs.h>

#include <stdio.h>

#include <PField.h>
#include <PFieldSet.h>

PFSCHED::PFSCHED() : Instrument()
{
}

PFSCHED::~PFSCHED()
{
}

int PFSCHED::init(double p[], int n_args)
{
// about ths RTBUFSAMPS+1 nonsense:
// 	when a PFSCHED note finishes, it de-allocates the PFields associated
// 	with it.  This is generally not a problem (because the 'connected'
// 	instrument/note drawing from the PField only needs the PField for
// 	the duration specified in the PFSCHED note), but if the duration is
// 	less than one RTBUFSAMP then the PFSCHED note will finish and deallocate
// 	*before* the other instrument gets a chance to read through the
// 	'connected' PField.  Instead of rewriting a huge amount of the PField
// 	code to handle the ref counters, etc. I just make sure that the
// 	PFSCHED note will last at least one buffer longer than the time
// 	computed for the PField to be read.  Almost no additional load
// 	on the CPU, see the run() method below.

	if (rtsetoutput(p[0], p[1]+((double)(RTBUFSAMPS+1)/SR), this) == -1)
		return DONT_SCHEDULE;

	pfbus = p[2];

	pfbusses[pfbus].drawflag = 0; // the 'connected' note will read when == 1
//	pfbusses[pfbus].thepfield = &((*_pfields)[3]);
	pfbusses[pfbus].thepfield = &(getPField(3)); // this is the PField to read
	pfbusses[pfbus].percent = 0.0;
	pfbusses[pfbus].theincr = (SR/(float)resetval)/(double)(nSamps()-(RTBUFSAMPS+1));
	// note the subtraction above, thus the PField will be read for the
	// correct duration

	return nSamps();
}

void PFSCHED::doupdate()
{
}

int PFSCHED::run()
{
	int i;

	pfbusses[pfbus].drawflag = 1; // signal to start reading from the pfield
	i = framesToRun();
	increment(i);

	return i;
}

Instrument*
makePFSCHED()
{
	PFSCHED *inst;
	inst = new PFSCHED();
	inst->set_bus_config("PFSCHED");
	return inst;
}

void
rtprofile()
{
	RT_INTRO("PFSCHED",makePFSCHED);
}
