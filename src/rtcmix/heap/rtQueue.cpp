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

// Push an element to end of the RTQueue

void RTQueue::push(Instrument *inInst, FRAMETYPE chunkstart)
{
#ifdef DEBUG_SORT
	mSorted = false;
#endif
	mInstrumentList.push_back(Element(chunkstart, inInst));
	inInst->ref();
}

bool RTQueue::sortElems (const RTQueue::Element& x,const RTQueue::Element& y)
{
	return (x.first > y.first);	// we pop from the end, so high frame numbers are in front
}

void RTQueue::sort()
{
#ifdef DEBUG_SORT
	if (mSorted) { fprintf(stderr, "SORTING RTQueue TWICE!\n"); }
#endif
	std::sort(mInstrumentList.begin(), mInstrumentList.end(), sortElems);
#ifdef DEBUG_SORT
	mSorted = true;
#endif
}

// Pop an element of the top of the RTQueue

Instrument *	RTQueue::pop(FRAMETYPE *pChunkStart)
{
#ifdef DEBUG_SORT
	assert(mSorted == true);
#endif
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



