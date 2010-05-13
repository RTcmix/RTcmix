/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
  the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <PFBusPField.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

PFBusPField::PFBusPField(
			const int			n_pfbus,
			const double		defaultval)
	: RTNumberPField(0),
	  _n_pfbus(n_pfbus)
{
	pfbusses[n_pfbus].val = defaultval;
	pfbusses[n_pfbus].percent = 0.0;
}

PFBusPField::~PFBusPField() {}

double PFBusPField::doubleValue(double dummy) const
{
	// the 'percent' struct member is set up in the PFSCHED instrument
	// to cover the appropriate duration of the PField
	if ( (pfbusses[_n_pfbus].drawflag == 1) && (pfbusses[_n_pfbus].percent < 1.0) )  {
		pfbusses[_n_pfbus].val = (*(pfbusses[_n_pfbus].thepfield)).doubleValue(pfbusses[_n_pfbus].percent);
		pfbusses[_n_pfbus].percent += pfbusses[_n_pfbus].theincr;

		if (pfbusses[_n_pfbus].percent >= 1.0) { // continue to read last value
			pfbusses[_n_pfbus].val = (*(pfbusses[_n_pfbus].thepfield)).doubleValue(1.0);
			// PField end, and dqflag is set, signal Instrument.cpp to de-queue
			if (pfbusses[_n_pfbus].dqflag == 1) {
				do_dq = 1;
				pfbusses[_n_pfbus].drawflag = 0; // set in case other PFSCHEDs
			}
		}
	}
	
	return pfbusses[_n_pfbus].val;
}

