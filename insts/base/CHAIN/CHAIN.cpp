/*	Runs a series of instruments as a group, chaining the output of each to the
    input of the next.  All the instruments are scheduled together.

		p0 		= output start time
		p1		= duration (-endtime)
 		p2		= number of instruments to follow
 		p3-n	= handles for instruments to be chained

 	To add an instruments to CHAIN, you have to create the instruments using the makeinstrument() utility.
 	Use the handle that is returned as the argument for that instrument.
 
 	CHAIN'd instruments do not use mix buses between them, but you still need to use bus_config() to configure
 	each instrument's input and output channel counts.  To do this, there is a special bus type called "chain",
 	which configures the instrument without invoking any of the in, out, or aux bus logic.  So, if you wished
 	to place WAVETABLE and DELAY in a CHAIN, you could configure each instrument like so:
 
 	bus_config("WAVETABLE", "chain 0 out");				// run WAVETABLE in monaural mode
 	bus_config("DELAY", "chain 0 in", "chain 1-2 out");	// run DELAY 1-channel in, 2-channel out
 	bus_config("CHAIN", "out 0-1");						// CHAIN's output MUST match output of last inst in chain (2-chan)
 
 	If the first instrument in the chain reads from disk, its input bus is configured just the way it would be in
 	an unchained system:
 
 	bus_config("TRANS", "in 0", "chain 0-1 out");		// read from file input, 2-channel out
 	bus_config("DELAY", "chain 0-1 in", "chain 2-3 out");	// run DELAY 2-channel in, 2-channel out
 	bus_config("CHAIN", "out 0-1");
 
 	In the same fashion as with aux busses, the outskip values for the CHAIN'd instruments should all be 0.
 	The CHAIN instrument alone determines the output skip.  The duration of the instruments can be arbitrary,
 	but will be truncated to the duration set in CHAIN.  If CHAIN's duration is longer than its instruments,
 	the extra time will be filled with zeros.

*/

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include <PFieldSet.h>
#include <rt.h>
#include <rtdefs.h>
#include <RTOption.h>
#include <MMPrint.h>
#include "CHAIN.h"


CHAIN::CHAIN()
{
}

void unrefInstrument(Instrument *i)
{
    i->unref();
}

CHAIN::~CHAIN()
{
    std::for_each(mInstVector.begin(), mInstVector.end(), unrefInstrument);
}

int  CHAIN::setup(PFieldSet *inPFields)
{
    if (inPFields->size() < 4) {
        return die("CHAIN", "Usage: CHAIN(outskip, duration, num_chained_instruments, instrument0 {, instrument1, ...}");
    }
    // Copy first 3 numeric args to new PFieldSet
	PFieldSet *newSet = new PFieldSet(3);
	for (int p = 0; p < 3; ++p)
		newSet->load(new ConstPField((*inPFields)[p].doubleValue(0.0)), p);
	// The remaining args are InstPFields
	int numChainedInstruments = (int)(*inPFields)[2].intValue(0.0);
	if (numChainedInstruments <= 0) {
		return die("CHAIN", "You must pass at least one instrument");
	}
	for (int p = 0; p < numChainedInstruments; ++p) {
		InstPField *ipf = (InstPField *) &(*inPFields)[p+3];
		Instrument *inst = ipf->instrument();
		if (inst == NULL) {
			return die("CHAIN", "NULL instrument passed as argument %d", p+3);
		}
		mInstVector.push_back(inst);
		// Instruments are referenced once when created.  Because CHAIN is the sole owner,
		// we do not do another reference.
	}
	if (RTOption::print() >= MMP_PRINTALL) {

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
    assert(previous != NULL);   // should not be possible for this to fire
	// For CHAIN itself, we override our (what should be zero) input channel count here.  This allows setChainedInputBuffer() to succeed
	// even though the counts don't seem to match.
	_input.inputchans = previous->outputChannels();
	status = setChainedInputBuffer(previous->outbuf, previous->outputChannels());
	return status;
}


int CHAIN::run()
{
//    printf("CHAIN::run(%p)\n", this);
    bool anInstRan = false;
	for (std::vector<Instrument *>::iterator it = mInstVector.begin(); it != mInstVector.end(); ++it) {
		Instrument *inst = *it;
		if (!inst->isDone()) {
			inst->setchunk(framesToRun());	// For outer instrument, this is done in inTraverse()
 //           printf("   CHAIN: running inst %p\n", inst);
            inst->run(true);
            anInstRan = true;
		}
		else {
 //           printf("   CHAIN: inst %p is done\n", inst);
			inst->clearOutput(framesToRun());			// This should be optimized to happen only once
		}
		inst->addout(BUS_NONE_OUT, 0);		// Special bus type makes this a no-op
	}
//    if (!anInstRan) {
//        printf("CHAIN::run(%p) - no internal instruments ran!\n", this);
//    }
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

#ifndef EMBEDDED
void rtprofile()
{
   RT_INTRO("CHAIN",makeCHAIN);
}
#endif
