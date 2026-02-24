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
#include <algorithm>

#include "dbug.h"


/* -------------------------- InstrumentBusManager::InstrumentBusManager --- */

InstrumentBusManager::InstrumentBusManager(int busCount, int bufsamps)
    : mBufsamps(bufsamps)
{
    (void)busCount;  /* map doesn't need pre-sizing */
}


/* ------------------------- InstrumentBusManager::~InstrumentBusManager --- */

InstrumentBusManager::~InstrumentBusManager()
{
    std::for_each(mInstBuses.begin(), mInstBuses.end(),
        [](std::pair<const int, InstrumentBus*> &p) { delete p.second; });
}


/* ------------------------ InstrumentBusManager::getOrCreateInstBus --- */

InstrumentBus* InstrumentBusManager::getOrCreateInstBus(int busID)
{
    assert(busID >= 0);

    std::map<int, InstrumentBus*>::iterator it = mInstBuses.find(busID);
    if (it != mInstBuses.end())
        return it->second;

#ifdef IBUG
    printf("InstBusMgr: creating InstrumentBus for bus %d\n", busID);
#endif
    /* aux_buffer is already allocated by bus_config.cpp.
     * For non-persistent mode (MULTIPLIER == 1), size is correct.
     * For persistent mode (MULTIPLIER > 1), bus_config.cpp would need
     * to be modified to allocate larger buffers.
     */
    InstrumentBus *ib = new InstrumentBus(busID, mBufsamps);
    mInstBuses[busID] = ib;
    return ib;
}


/* ------------------------------- InstrumentBusManager::getInstBus --- */

InstrumentBus* InstrumentBusManager::getInstBus(int busID) const
{
    std::map<int, InstrumentBus*>::const_iterator it = mInstBuses.find(busID);
    return (it != mInstBuses.end()) ? it->second : NULL;
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


/* ----------------------- InstrumentBusManager::removeConsumer --- */

void InstrumentBusManager::removeConsumer(Instrument* inst)
{
    std::for_each(mInstBuses.begin(), mInstBuses.end(),
        [inst](std::pair<const int, InstrumentBus*> &p) { p.second->removeConsumer(inst); });
}


/* ----------------------- InstrumentBusManager::advanceAllProduction --- */

void InstrumentBusManager::advanceAllProduction(int frames, FRAMETYPE currentBufStart)
{
    std::for_each(mInstBuses.begin(), mInstBuses.end(),
        [frames, currentBufStart](std::pair<const int, InstrumentBus*> &p) {
            p.second->advanceProduction(frames, currentBufStart);
        });
}


/* ----------------------------------- InstrumentBusManager::reset --- */

void InstrumentBusManager::reset()
{
    std::for_each(mInstBuses.begin(), mInstBuses.end(),
        [](std::pair<const int, InstrumentBus*> &p) { p.second->reset(); });

#ifdef IBUG
    printf("InstBusMgr: reset %d active InstrumentBus objects\n", (int)mInstBuses.size());
#endif
}


/* ------------------------------------------- RTcmix::getInstBus --- */

InstrumentBus* RTcmix::getInstBus(int busID)
{
    return instBusManager->getInstBus(busID);
}
