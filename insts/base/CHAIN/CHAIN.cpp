/*	Run a series of instruments as a group, chaining the output of each to the 
    input of the next.

		p0 = output start time
		p1 = duration (-endtime)

*/
#include <unistd.h>
#include <stdio.h>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include <PFieldSet.h>
#include <rt.h>
#include <rtdefs.h>
#include <Option.h>
#include <MMPrint.h>
#include "CHAIN.h"


CHAIN::CHAIN()
{
}

CHAIN::~CHAIN()
{
	for (std::vector<Instrument *>::iterator it = mInstVector.begin(); it != mInstVector.end(); ++it) {
		Instrument *inst = *it;
		inst->unref();
	}
}

int  CHAIN::setup(PFieldSet *inPFields)
{
	// Copy first 3 numeric args to new PFieldSet
	PFieldSet *newSet = new PFieldSet(3);
	for (int p = 0; p < 3; ++p)
		newSet->load(new ConstPField((*inPFields)[p].doubleValue(0.0)), p);
	// The remaining args are InstPFields
	int numChainedInstruments = (int)(*inPFields)[2].intValue(0.0);
	for (int p = 0; p < numChainedInstruments; ++p) {
		InstPField *ipf = (InstPField *) &(*inPFields)[p+3];
		Instrument *inst = ipf->instrument();
		mInstVector.push_back(inst);
		inst->ref();
	}
	if (Option::print() >= MMP_PRINTALL) {

		RTPrintfCat("Instrument chain: ");
		for (std::vector<Instrument *>::iterator it = mInstVector.begin(); it != mInstVector.end(); ++it) {
			RTPrintfCat("%s -> ", (*it)->name());
		}
		RTPrintf("Out\n");
	}
	delete inPFields;
	return Instrument::setup(newSet);
}

int CHAIN::init(double p[], int n_args)
{
	const float outskip = p[0];
	float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	return nSamps();
}


int CHAIN::configure()
{
	int status = -1;
	Instrument *previous = NULL;
	for (std::vector<Instrument *>::iterator it = mInstVector.begin(); it != mInstVector.end(); ++it) {
		Instrument *inst = *it;
		status = inst->configure(RTBUFSAMPS);
		if (status != 0)
			return status;
		if (previous != NULL) {
			status = inst->setChainedInputBuffer(previous->outbuf, previous->outputChannels());
			if (status != 0)
				return status;
		}
		previous = inst;
	}
	// For CHAIN itself, we override our (what should be zero) input channel count here.  This allows setChainedInputBuffer() to succeed
	// even though the counts don't seem to match.
	_input.inputchans = previous->outputChannels();
	status = setChainedInputBuffer(previous->outbuf, previous->outputChannels());
	return status;
}


int CHAIN::run()
{
	for (std::vector<Instrument *>::iterator it = mInstVector.begin(); it != mInstVector.end(); ++it) {
		Instrument *inst = *it;
		inst->setchunk(framesToRun());	// For outer instrument, this is done in inTraverse()
		inst->run(true);
		inst->addout(BUS_NONE_OUT, 0);		// Special bus type makes this a no-op
	}
	// Copy from inputChainedBuf, which points to the outbuf of the last instrument in the chain.
	unsigned copySize = framesToRun() * outputChannels() * sizeof(BUFTYPE);
	memcpy(outbuf, inputChainBuf, copySize);
	return framesToRun();
}


Instrument *makeCHAIN()
{
	CHAIN *inst = new CHAIN();
	inst->set_bus_config("CHAIN");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("CHAIN",makeCHAIN);
}
#endif
