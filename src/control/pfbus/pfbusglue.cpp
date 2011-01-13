/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <utils.h>   // in ../../rtcmix
#include <PFBusPField.h>
#include <ugens.h>		// for warn, die


// -----------------------------------------------------------------------------
//
//		stream = makeconnection("pfbus", pfbus_no, default);
//
//		<pfbus_no>		the max object inlet number associated with this handle
//    <default>      default value (delivered while a PFSCHED() instrument
//								hasn't started executing yet)
//
//		score to use this might be:
//
//		delayed_envelope = maketable("line", 1000, 0,0.0,  1,1.0)
//		PFSCHED(3.5, 2.1, 1, delayed_envelope)
//		value = makeconnection("pfbus", 1, 0.0)
//		INSTRUMENT(0, 5.6, value, p3)
//
//		the "value" PField for INSTRUMENT will be 0.0 until time 3.5, then
//		the delayed_envelope table will start delivering values for 2.1 seconds
//		through the PFSCHED instrument.  The values will be set on pfbus #1
//		(pfbusses[1] internally).
//				BGG 11/2009

// BGG mm -- these are in insts/bgg/PFSCHED.h; catch a few crashes
extern struct pfbusdata pfbusses[];
extern int pfbus_is_connected[];


static RTNumberPField *
_pfbus_usage()
{
	die("makeconnection (pfbus)",
		"Usage: makeconnection(\"pfbus\", pfbus #, default)");
	return NULL;
}

static RTNumberPField *
create_pfield(const Arg args[], const int nargs)
{
	int pfbusval;
	double defaultval;

	if (nargs < 2)
		return _pfbus_usage();

	if (args[0].isType(DoubleType))
		pfbusval = (int)(args[0]);
	else
		return _pfbus_usage();

	if (args[1].isType(DoubleType))
		defaultval = args[1];
	else
		return _pfbus_usage();

	if (pfbusses[pfbusval].thepfield != NULL) {
		rterror("pfbusses", "pfbus %d is already connected", pfbusval);
		return (RTNumberPField*)(pfbusses[pfbusval].thepfield);
	}
	pfbus_is_connected[pfbusval] = 1;

	return new PFBusPField(pfbusval, defaultval);
}

// The following functions are the publically-visible ones called by the
// system.

extern "C" {
	Handle create_handle(const Arg args[], const int nargs);
	int register_dso();
};

Handle
create_handle(const Arg args[], const int nargs)
{
	PField *pField = create_pfield(args, nargs);
	Handle handle = NULL;
	if (pField != NULL) {
		handle = createPFieldHandle(pField);
	}
	return handle;
}

int register_dso()
{
   return 0;
}

