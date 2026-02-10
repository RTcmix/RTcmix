/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * InstrumentBusManager.cpp - InstrumentBus management implementation
 *
 * See InstrumentBusManager.h for design overview.
 */

#include "InstrumentBusManager.h"
#include "InstrumentBus.h"
#include "Instrument.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef MULTI_THREAD
#include "TaskManager.h"
#endif

/* Debug macros - match pattern from intraverse.cpp */
#undef BBUG      /* InstrumentBus-specific debugging */
#undef WBUG      /* "Where we are" prints */
#undef DBUG      /* General debug */
#undef ALLBUG    /* All debug output */

#ifdef ALLBUG
#define BBUG
#define WBUG
#define DBUG
#endif


/* -------------------------- InstrumentBusManager::InstrumentBusManager --- */

InstrumentBusManager::InstrumentBusManager(int busCount, int numChannels, int bufsamps)
    : mInstBuses(busCount, NULL),
      mNumChannels(numChannels),
      mBufsamps(bufsamps),
      mActiveInstBusCount(0)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING InstrumentBusManager::InstrumentBusManager(busCount=%d, chans=%d, bufsamps=%d)\n",
           busCount, numChannels, bufsamps);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBusManager::InstrumentBusManager()\n");
#endif
}


/* ------------------------- InstrumentBusManager::~InstrumentBusManager --- */

InstrumentBusManager::~InstrumentBusManager()
{
#ifdef WBUG
    printf("ENTERING InstrumentBusManager::~InstrumentBusManager()\n");
#endif

    /* Delete all InstrumentBus objects */
    for (size_t i = 0; i < mInstBuses.size(); ++i) {
        delete mInstBuses[i];
    }

#ifdef WBUG
    printf("EXITING InstrumentBusManager::~InstrumentBusManager()\n");
#endif
}


/* ------------------------ InstrumentBusManager::getOrCreateInstBus --- */

InstrumentBus* InstrumentBusManager::getOrCreateInstBus(int busID)
{
    assert(busID >= 0 && (size_t)busID < mInstBuses.size());

    if (mInstBuses[busID] == NULL) {
#ifdef BBUG
        printf("InstBusMgr: creating InstrumentBus for bus %d\n", busID);
#endif
        mInstBuses[busID] = new InstrumentBus(busID, mNumChannels, 0, mBufsamps);
        ++mActiveInstBusCount;

#ifdef MULTI_THREAD
        if (mTaskManager != NULL) {
            mInstBuses[busID]->setTaskManager(mTaskManager);
        }
#endif
    }

    return mInstBuses[busID];
}


/* ------------------------------- InstrumentBusManager::getInstBus --- */

InstrumentBus* InstrumentBusManager::getInstBus(int busID) const
{
    if (busID < 0 || (size_t)busID >= mInstBuses.size()) {
        return NULL;
    }
    return mInstBuses[busID];
}


/* ------------------------------ InstrumentBusManager::hasInstBus --- */

bool InstrumentBusManager::hasInstBus(int busID) const
{
    if (busID < 0 || (size_t)busID >= mInstBuses.size()) {
        return false;
    }
    return mInstBuses[busID] != NULL;
}


/* --------------------------------- InstrumentBusManager::addWriter --- */

void InstrumentBusManager::addWriter(int busID, Instrument* inst)
{
    InstrumentBus* instBus = getOrCreateInstBus(busID);
    instBus->addWriter(inst);

#ifdef BBUG
    printf("InstBusMgr: added writer %p [%s] to bus %d\n",
           inst, inst->name(), busID);
#endif
}


/* ------------------------------- InstrumentBusManager::addConsumer --- */

void InstrumentBusManager::addConsumer(int busID, Instrument* inst)
{
    InstrumentBus* instBus = getOrCreateInstBus(busID);
    instBus->addConsumer(inst);

    /* NOTE: Don't enable InstrumentBus path yet - intraverse.cpp integration not complete.
     * When Phase 2 is implemented, uncomment this line:
     * inst->setInputInstBus(instBus);
     * For now, instruments use the legacy aux_buffer path in rtgetin().
     */

#ifdef BBUG
    printf("InstBusMgr: added consumer %p [%s] to bus %d\n",
           inst, inst->name(), busID);
#endif
}


/* ------------------------------ InstrumentBusManager::removeWriter --- */

void InstrumentBusManager::removeWriter(int busID, Instrument* inst)
{
    if (busID >= 0 && (size_t)busID < mInstBuses.size() && mInstBuses[busID] != NULL) {
        mInstBuses[busID]->removeWriter(inst);

#ifdef BBUG
        printf("InstBusMgr: removed writer %p from bus %d\n", inst, busID);
#endif
    }
}


/* ----------------------------------- InstrumentBusManager::reset --- */

void InstrumentBusManager::reset()
{
#ifdef WBUG
    printf("ENTERING InstrumentBusManager::reset()\n");
#endif

    for (size_t i = 0; i < mInstBuses.size(); ++i) {
        if (mInstBuses[i] != NULL) {
            mInstBuses[i]->reset();
        }
    }

#ifdef BBUG
    printf("InstBusMgr: reset %d active InstrumentBus objects\n", mActiveInstBusCount);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBusManager::reset()\n");
#endif
}


/* ------------------------- InstrumentBusManager::getActiveInstBusCount --- */

int InstrumentBusManager::getActiveInstBusCount() const
{
    return mActiveInstBusCount;
}


#ifdef MULTI_THREAD
/* ------------------------------ InstrumentBusManager::setTaskManager --- */

void InstrumentBusManager::setTaskManager(TaskManager* tm)
{
    mTaskManager = tm;

    /* Update all existing InstrumentBus objects */
    for (size_t i = 0; i < mInstBuses.size(); ++i) {
        if (mInstBuses[i] != NULL) {
            mInstBuses[i]->setTaskManager(tm);
        }
    }

#ifdef BBUG
    printf("InstBusMgr: set TaskManager to %p\n", tm);
#endif
}
#endif