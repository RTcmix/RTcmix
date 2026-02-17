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
#include <stdio.h>
#include <assert.h>

#include "dbug.h"


/* -------------------------- InstrumentBusManager::InstrumentBusManager --- */

InstrumentBusManager::InstrumentBusManager(int busCount, int bufsamps)
    : mInstBuses(busCount, NULL),
      mBufsamps(bufsamps),
      mActiveInstBusCount(0)
{
}


/* ------------------------- InstrumentBusManager::~InstrumentBusManager --- */

InstrumentBusManager::~InstrumentBusManager()
{
    for (size_t i = 0; i < mInstBuses.size(); ++i) {
        delete mInstBuses[i];
    }
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


/* ----------------------------------- InstrumentBusManager::reset --- */

void InstrumentBusManager::reset()
{
    for (size_t i = 0; i < mInstBuses.size(); ++i) {
        if (mInstBuses[i] != NULL) {
            mInstBuses[i]->reset();
        }
    }

#ifdef IBUG
    printf("InstBusMgr: reset %d active InstrumentBus objects\n", mActiveInstBusCount);
#endif
}


/* ------------------------- InstrumentBusManager::getActiveInstBusCount --- */

int InstrumentBusManager::getActiveInstBusCount() const
{
    return mActiveInstBusCount;
}
