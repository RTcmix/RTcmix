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

void RTQueue::unrefElems(Element &e)
{
    e.second->unref();
}

RTQueue::~RTQueue()
{
    std::for_each(mInstrumentVector.begin(), mInstrumentVector.end(), unrefElems);
}

bool RTQueue::sortElems (const RTQueue::Element& x,const RTQueue::Element& y)
{
	return (x.first > y.first);
}

// Push an element to end of the RTQueue

void RTQueue::push(Instrument *inInst, FRAMETYPE chunkstart)
{
	Element element(chunkstart, inInst);
	mInstrumentVector.insert(std::lower_bound(mInstrumentVector.begin(), mInstrumentVector.end(), element, sortElems), element);
	inInst->ref();
}

// Push an element onto RTQueue, unsorted

void RTQueue::pushUnsorted(Instrument *inInst, FRAMETYPE chunkstart)
{
    Element element(chunkstart, inInst);
    mInstrumentVector.push_back(element);
    inInst->ref();
}

void RTQueue::sort()
{
    std::sort(mInstrumentVector.begin(), mInstrumentVector.end(), sortElems);
}

// Pop an element of the top of the RTQueue

Instrument *	RTQueue::pop(FRAMETYPE *pChunkStart)
{
	if (mInstrumentVector.empty()) {
		rtcmix_warn("rtQueue", "attempt to pop empty RTQueue\n");
		return NULL;
	}
	*pChunkStart = mInstrumentVector.back().first;	// frame loc
	Instrument *outInst = mInstrumentVector.back().second;	// inst
	outInst->unref();
	mInstrumentVector.pop_back();
	return outInst;
}

// Return the starting sample chunk of the top Instrument

FRAMETYPE RTQueue::nextChunk()
{
	return mInstrumentVector.back().first;
}

void RTQueue::print() {
	
}



