/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * TierManager.h - Manages the tier graph for pull-based audio routing
 *
 * The TierManager maintains all Tier objects and provides methods
 * for creating, accessing, and resetting tiers.
 *
 * Author: Claude Code / RTcmix Development Team
 */

#ifndef _TIERMANAGER_H_
#define _TIERMANAGER_H_

#include <rt_types.h>
#include <bus.h>

class Tier;
class Instrument;
class TaskManager;

/* Debug macro for tier manager operations */
#undef TBUG


class TierManager {
public:
    /**
     * Create a new TierManager.
     *
     * @param busCount     Maximum number of buses
     * @param numChannels  Number of audio channels per bus
     * @param bufsamps     Frames per buffer (RTBUFSAMPS)
     */
    TierManager(int busCount, int numChannels, int bufsamps);

    ~TierManager();

    /**
     * Get or create a tier for the specified aux bus.
     *
     * @param busID  The aux bus number
     * @return       Pointer to the tier (never NULL)
     */
    Tier* getOrCreateTier(int busID);

    /**
     * Get a tier for the specified aux bus (NULL if not yet created).
     *
     * @param busID  The aux bus number
     * @return       Pointer to the tier, or NULL if not created
     */
    Tier* getTier(int busID) const;

    /**
     * Check if a tier exists for the specified bus.
     *
     * @param busID  The aux bus number
     * @return       true if a tier exists
     */
    bool hasTier(int busID) const;

    /**
     * Register an instrument as a writer to a tier.
     *
     * @param busID  The aux bus number
     * @param inst   The instrument that writes to this bus
     */
    void addWriter(int busID, Instrument* inst);

    /**
     * Register an instrument as a consumer of a tier.
     *
     * @param busID  The aux bus number
     * @param inst   The instrument that reads from this bus
     */
    void addConsumer(int busID, Instrument* inst);

    /**
     * Remove a writer from a tier.
     *
     * @param busID  The aux bus number
     * @param inst   The instrument to remove
     */
    void removeWriter(int busID, Instrument* inst);

    /**
     * Reset all tiers for a new audio run.
     */
    void reset();

    /**
     * Get the number of tiers currently active.
     */
    int getActiveTierCount() const;

#ifdef MULTI_THREAD
    /**
     * Set the TaskManager for all tiers.
     *
     * @param tm  Pointer to the TaskManager
     */
    void setTaskManager(TaskManager* tm);
#endif

private:
    Tier** mTiers;        /* Array of tier pointers (indexed by bus ID) */
    int mBusCount;        /* Maximum number of buses */
    int mNumChannels;     /* Channels per bus */
    int mBufsamps;        /* Frames per buffer */
    int mActiveTierCount; /* Number of active tiers */

#ifdef MULTI_THREAD
    TaskManager* mTaskManager;
#endif

    /* Prevent copying */
    TierManager(const TierManager&);
    TierManager& operator=(const TierManager&);
};

#endif /* _TIERMANAGER_H_ */