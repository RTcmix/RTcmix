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
	p4 = [optional] flag to set instrument to de-queue itself at end
		of the duration

	BGG, 11/2009
*/

#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include "PFSCHED.h"
#include <rt.h>
#include <rtdefs.h>

#include <stdio.h>
#include <string.h> // for bzero()

#include <PField.h>
#include <PFieldSet.h>

// these are to enable dynamic maketable() construction
#include "../../../src/include/maxdispargs.h"
#include "../../../src/rtcmix/rtcmix_types.h"
int _dispatch_table(const Arg *args, const int nargs, const int startarg, double **array, int *len);
Handle createPFieldHandle(class PField *);


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

	// if set_dq_flag is 1, then the dqflag of pfbusses[] will be set to
	// signal de-queuing at end of this duration/envelope
	if (n_args > 4) set_dq_flag = 1;
	else set_dq_flag = 0;

	pfbus = p[2];

	pfbusses[pfbus].drawflag = 0; // the 'connected' note will read when == 1
//	pfbusses[pfbus].thepfield = &((*_pfields)[3]);
//	pfbusses[pfbus].thepfield = &(getPField(3)); // this is the PField to read
	PFSCHEDpfield = &(getPField(3)); // this is the PField to read

	// set the other fields in ::run in case multiple PFSCHEDs on one pfbus
	firsttime = 1;

	return nSamps();
}

int PFSCHED::configure()
{
	bzero((void *)outbuf, (RTBUFSAMPS * NCHANS)*sizeof(BUFTYPE));
   return 0;
}


void PFSCHED::doupdate()
{
}

int PFSCHED::run()
{
	int i;

	if (firsttime == 1) {
		double tval = 0.0; // get rid of the compiler warning :-)
		if (pfbusses[pfbus].thepfield != NULL) tval = pfbusses[pfbus].val;

		pfbusses[pfbus].thepfield = PFSCHEDpfield; // this is the PField to read
		Arg targs[MAXDISPARGS];
		int nargs, lenindex;

		// if DYNTABLETOKEN is the first value in the data, it means we need
		// to construct a new table, based on the current pfield value
		// and the construction data stored in the data[] array
		if ( (*(pfbusses[pfbus].thepfield)).doubleValue(0) == DYNTABLETOKEN) {
			nargs = (int)(*(pfbusses[pfbus].thepfield)).doubleValue(1) + 2;
			lenindex = 1;
			targs[0] = "line";
			targs[1] = (*(pfbusses[pfbus].thepfield)).doubleValue(2);

			for (i = 2; i < nargs; i++) {
				// additional occurrences of DYNTABLETOKEN signal a 'curval', so
				// use the current pfield value
				if ( (*(pfbusses[pfbus].thepfield)).doubleValue(i+1) == DYNTABLETOKEN) {
					targs[i] = tval;
				} else {
					targs[i] = (*(pfbusses[pfbus].thepfield)).doubleValue(i+1);
				}
			}

			int len = targs[lenindex];
			double *data = NULL;
			data = new double[len];
			if (data == NULL) {
				die("maketable", "Out of memory.");
				return NULL;
			}

			if (_dispatch_table(targs, nargs, lenindex + 1, &data, &len) != 0) {
				delete [] data;
				return NULL;            // error message already given
			}

			TablePField *table;
			table = new TablePField(data, len);

			// replace the pfield with the newly-constructed table
			pfbusses[pfbus].thepfield = table;
		}

		pfbusses[pfbus].percent = 0.0;
		pfbusses[pfbus].dqflag = 0;
		// note the subtraction; the PField will be read for the correct duration
		pfbusses[pfbus].theincr = (SR/(float)resetval)/(double)(nSamps()-(RTBUFSAMPS+1));
		pfbusses[pfbus].drawflag = 1; // signal to start reading from the pfield
		firsttime = 0;
	}

	if (set_dq_flag == 1) pfbusses[pfbus].dqflag = 1;
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
