/* PFSCHED - 'vehicle' instrument to allow schedule PField values

	PFSCHED takes a PField as an input and will read from it
	at a scheduled time for a scheduled duration.  These values
	are then written to a 'pfield bus' specified in a makeconnection()
	scorefile cammand.  The PField handle from that makeconnection()
	can then be used to feed scheduled PField values into an
	executing instrument

	p0 = output start time (NOTE: only outskip of 0.0 is now allowed (12/2012))
	p1 = duration
	p2 = 'pfield bus' to use
	p3 = input PField variable
	p4 = [optional] flag to set instrument to de-queue itself at end
		of the duration

	BGG, 11/2009
	BGG, 12/2012 -- fixed de-queuing problem, changed to PFBusdata approach
*/


#include <stdlib.h>
#include <Instrument.h>
#include <ugens.h>
#include <PField.h>
#include <PFBusData.h>
#include "PFSCHED.h"
#include <rt.h>
#include <rtdefs.h>
#include <stdio.h>
#include <string.h> // for bzero()

// these are to enable dynamic maketable() construction
#include "../../../src/include/maxdispargs.h"
#include "../../../src/rtcmix/rtcmix_types.h"
int _dispatch_table(const Arg *args, const int nargs, const int startarg, double **array, int *len);
Handle createPFieldHandle(class PField *);

// BGG -- this is from src/rtcmix/table.cpp, but renamed slightly in case
// of static lib conflicts.  Tried an extern declaration but it didn't work
static const char *_table_names[] = {
   "textfile", // 0
   "soundfile",
   "literal",
   "datafile",
   "curve",    // 4
   "expbrk",
   "line",
   "linebrk",
   "spline",   // 8
   "wave3",
   "wave",
   NULL,
   NULL,       // 12
   NULL,
   NULL,
   NULL,
   NULL,       // 16
   "cheby",
   "line",
   NULL,
   "random",   // 20
   NULL,
   NULL,
   NULL,
   "line",     // 24
   "window",
   "linestep",
   NULL
};


PFSCHED::PFSCHED() : Instrument()
{
}

PFSCHED::~PFSCHED()
{
}

int PFSCHED::init(double p[], int n_args)
{
	if (p[0] != 0.0) {
		rterror("PFSCHED", "PFSCHED start time has to be 0.0");
		return DONT_SCHEDULE;
	}

	pfbus = p[2];

	if (PFBusData::pfbus_is_connected[pfbus] != 1) {
		rterror("PFSCHED", "pfbus %d not connected", pfbus);
		return DONT_SCHEDULE;
	}

	if (rtsetoutput(p[0], p[1], this) == -1)
		return DONT_SCHEDULE;

	PFBusData::dq_now[pfbus] = 0;
	PFBusData::drawflag[pfbus]  = 0; // prevent reading from the bus for now
	PFBusData::thepfield[pfbus] = &(getPField(3)); // the PField to read
	makedyntable();
	PFBusData::dqflag[pfbus]  = p[4];
	PFBusData::theincr[pfbus] = (SR/(float)resetval)/(double)(nSamps());
	PFBusData::percent[pfbus] = 0.0;
	PFBusData::drawflag[pfbus] = 1; // signal to start reading from the pfield

	return nSamps();
}

int PFSCHED::makedyntable()
{
	int i;
	double tval = 0.0; // get rid of the compiler warning :-)

	const PField *PF = PFBusData::thepfield[pfbus];
	if (PF != NULL) tval = PFBusData::val[pfbus];

	Arg targs[MAXDISPARGS];
	int nargs, lenindex;

	// if DYNTABLETOKEN is the first value in the data, it means we need
	// to construct a new table, based on the current pfield value
	// and the construction data stored in the data[] array
	// see src/rtcmix/table.cpp for the spec for the data in the data[] array
	if ( (*PF).doubleValue(0) == DYNTABLETOKEN) {
		nargs = (int)(*PF).doubleValue(2) + 2; // counting past targs[0] and [1]
		targs[0] = _table_names[(int)(*PF).doubleValue(1)];
		targs[1] = (*PF).doubleValue(3);
		lenindex = 1; // this indexing the targs[] array for _dispatch_table()

		for (i = 2; i < nargs; i++) {
			// additional occurrences of DYNTABLETOKEN signal a 'curval', so
			// use the current pfield value
			if ( (*PF).doubleValue(i+2) == DYNTABLETOKEN ) {
				targs[i] = tval;
			} else {
				targs[i] = (*PF).doubleValue(i+2);
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
		PFBusData::thepfield[pfbus] = table;
	}

	return(1);
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


#ifndef MAXMSP
void
rtprofile()
{
	RT_INTRO("PFSCHED",makePFSCHED);
}
#endif
