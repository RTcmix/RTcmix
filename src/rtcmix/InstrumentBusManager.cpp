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
#include <RTcmix.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef MULTI_THREAD
#include "TaskManager.h"
#endif

/* Debug macros - match pattern from intraverse.cpp */
#undef BBUG      /* Verbose bus debugging */
#undef WBUG      /* "Where we are" prints */
#undef IBUG      /* Instrument and InstrumentBus debugging */
#undef DBUG      /* General debug */
#undef ALLBUG    /* All debug output */

#ifdef ALLBUG
#define BBUG
#define WBUG
#define IBUG
#define DBUG
#endif


/* -------------------------- InstrumentBusManager::InstrumentBusManager --- */

InstrumentBusManager::InstrumentBusManager(int busCount, int bufsamps)
    : mInstBuses(busCount, NULL),
      mBufsamps(bufsamps),
      mActiveInstBusCount(0)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING InstrumentBusManager::InstrumentBusManager(busCount=%d, bufsamps=%d)\n",
           busCount, bufsamps);
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
#ifdef IBUG
        printf("InstBusMgr: creating InstrumentBus for bus %d\n", busID);
#endif
        /* aux_buffer is already allocated by bus_config.cpp.
         * For non-persistent mode (MULTIPLIER == 1), size is correct.
         * For persistent mode (MULTIPLIER > 1), bus_config.cpp would need
         * to be modified to allocate larger buffers.
         */
        mInstBuses[busID] = new InstrumentBus(busID, mBufsamps);
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

#ifdef IBUG
    printf("InstBusMgr: added writer %p [%s] to bus %d\n",
           inst, inst->name(), busID);
#endif
}


/* ------------------------------- InstrumentBusManager::addConsumer --- */

void InstrumentBusManager::addConsumer(int busID, Instrument* inst)
{
    InstrumentBus* instBus = getOrCreateInstBus(busID);
    instBus->addConsumer(inst);

#ifdef IBUG
    printf("InstBusMgr: added consumer %p [%s] to bus %d\n",
           inst, inst->name(), busID);
#endif
}


/* ------------------------------ InstrumentBusManager::removeWriter --- */

void InstrumentBusManager::removeWriter(int busID, Instrument* inst)
{
    if (busID >= 0 && (size_t)busID < mInstBuses.size() && mInstBuses[busID] != NULL) {
        mInstBuses[busID]->removeWriter(inst);

#ifdef IBUG
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

#ifdef IBUG
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

#ifdef IBUG
    printf("InstBusMgr: set TaskManager to %p\n", tm);
#endif
}
#endif