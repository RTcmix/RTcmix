/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * TierManager.cpp - Tier graph management implementation
 *
 * See TierManager.h for design overview.
 */

#include "TierManager.h"
#include "Tier.h"
#include "Instrument.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef MULTI_THREAD
#include "TaskManager.h"
#endif

/* Debug macros - match pattern from intraverse.cpp */
#undef TBUG      /* Tier-specific debugging */
#undef WBUG      /* "Where we are" prints */
#undef DBUG      /* General debug */
#undef ALLBUG    /* All debug output */

#ifdef ALLBUG
#define TBUG
#define WBUG
#define DBUG
#endif


/* ---------------------------------------------------- TierManager::TierManager --- */

TierManager::TierManager(int busCount, int numChannels, int bufsamps)
    : mTiers(NULL),
      mBusCount(busCount),
      mNumChannels(numChannels),
      mBufsamps(bufsamps),
      mActiveTierCount(0)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING TierManager::TierManager(busCount=%d, chans=%d, bufsamps=%d)\n",
           busCount, numChannels, bufsamps);
#endif

    /* Allocate tier pointer array (all NULL initially) */
    mTiers = new Tier*[mBusCount];
    memset(mTiers, 0, sizeof(Tier*) * mBusCount);

#ifdef WBUG
    printf("EXITING TierManager::TierManager()\n");
#endif
}


/* --------------------------------------------------- TierManager::~TierManager --- */

TierManager::~TierManager()
{
#ifdef WBUG
    printf("ENTERING TierManager::~TierManager()\n");
#endif

    /* Delete all tiers */
    for (int i = 0; i < mBusCount; ++i) {
        delete mTiers[i];
    }
    delete[] mTiers;

#ifdef WBUG
    printf("EXITING TierManager::~TierManager()\n");
#endif
}


/* ---------------------------------------------- TierManager::getOrCreateTier --- */

Tier* TierManager::getOrCreateTier(int busID)
{
    assert(busID >= 0 && busID < mBusCount);

    if (mTiers[busID] == NULL) {
#ifdef TBUG
        printf("TierManager: creating tier for bus %d\n", busID);
#endif
        mTiers[busID] = new Tier(busID, mNumChannels, 0, mBufsamps);
        ++mActiveTierCount;

#ifdef MULTI_THREAD
        if (mTaskManager != NULL) {
            mTiers[busID]->setTaskManager(mTaskManager);
        }
#endif
    }

    return mTiers[busID];
}


/* ----------------------------------------------------- TierManager::getTier --- */

Tier* TierManager::getTier(int busID) const
{
    if (busID < 0 || busID >= mBusCount) {
        return NULL;
    }
    return mTiers[busID];
}


/* ---------------------------------------------------- TierManager::hasTier --- */

bool TierManager::hasTier(int busID) const
{
    if (busID < 0 || busID >= mBusCount) {
        return false;
    }
    return mTiers[busID] != NULL;
}


/* --------------------------------------------------- TierManager::addWriter --- */

void TierManager::addWriter(int busID, Instrument* inst)
{
    Tier* tier = getOrCreateTier(busID);
    tier->addWriter(inst);

#ifdef TBUG
    printf("TierManager: added writer %p [%s] to bus %d\n",
           inst, inst->name(), busID);
#endif
}


/* ------------------------------------------------- TierManager::addConsumer --- */

int TierManager::addConsumer(int busID, Instrument* inst)
{
    Tier* tier = getOrCreateTier(busID);
    int consumerID = tier->addConsumer();

    /* Configure the instrument with its tier and consumer ID */
    inst->setInputTier(tier, consumerID);

#ifdef TBUG
    printf("TierManager: added consumer %p [%s] to bus %d with consumerID %d\n",
           inst, inst->name(), busID, consumerID);
#endif

    return consumerID;
}


/* ------------------------------------------------ TierManager::removeWriter --- */

void TierManager::removeWriter(int busID, Instrument* inst)
{
    if (busID >= 0 && busID < mBusCount && mTiers[busID] != NULL) {
        mTiers[busID]->removeWriter(inst);

#ifdef TBUG
        printf("TierManager: removed writer %p from bus %d\n", inst, busID);
#endif
    }
}


/* ----------------------------------------------------- TierManager::reset --- */

void TierManager::reset()
{
#ifdef WBUG
    printf("ENTERING TierManager::reset()\n");
#endif

    for (int i = 0; i < mBusCount; ++i) {
        if (mTiers[i] != NULL) {
            mTiers[i]->reset();
        }
    }

#ifdef TBUG
    printf("TierManager: reset %d active tiers\n", mActiveTierCount);
#endif

#ifdef WBUG
    printf("EXITING TierManager::reset()\n");
#endif
}


/* ----------------------------------------- TierManager::getActiveTierCount --- */

int TierManager::getActiveTierCount() const
{
    return mActiveTierCount;
}


#ifdef MULTI_THREAD
/* -------------------------------------------- TierManager::setTaskManager --- */

void TierManager::setTaskManager(TaskManager* tm)
{
    mTaskManager = tm;

    /* Update all existing tiers */
    for (int i = 0; i < mBusCount; ++i) {
        if (mTiers[i] != NULL) {
            mTiers[i]->setTaskManager(tm);
        }
    }

#ifdef TBUG
    printf("TierManager: set TaskManager to %p\n", tm);
#endif
}
#endif