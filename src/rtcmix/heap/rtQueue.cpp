/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include "heap.h"
#include "dbug.h"
#include <rtdefs.h>
#include <RTcmix.h>
#include <Instrument.h>
#include <ugens.h> // for rtcmix_warn()
#include <algorithm>
#include <assert.h>

RTQueue::~RTQueue()
{
	for (InstrumentListIterator it = mInstrumentList.begin();
		 it != mInstrumentList.end();
		 ++it) {
		(*it).second->unref();
	}
}

bool RTQueue::sortElems (const RTQueue::Element& x,const RTQueue::Element& y)
{
	return (x.first < y.first);
}

// Push an element to end of the RTQueue

void RTQueue::push(Instrument *inInst, FRAMETYPE chunkstart)
{
	Element element(chunkstart, inInst);
	mInstrumentList.insert(std::upper_bound(mInstrumentList.begin(), mInstrumentList.end(), element, sortElems), element);
	inInst->ref();
}

// Pop an element of the top of the RTQueue

Instrument *	RTQueue::pop(FRAMETYPE *pChunkStart)
{
	if (mInstrumentList.empty()) {
		rtcmix_warn("rtQueue", "attempt to pop empty RTQueue\n");
		return NULL;
	}
	*pChunkStart = mInstrumentList.back().first;	// frame loc
	Instrument *outInst = mInstrumentList.back().second;	// inst
	outInst->unref();
	mInstrumentList.pop_back();
	return outInst;
}

// Return the starting sample chunk of the top Instrument

FRAMETYPE RTQueue::nextChunk()
{
	return mInstrumentList.back().first;
}

void RTQueue::print() {
	
}



