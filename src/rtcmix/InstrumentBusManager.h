/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * InstrumentBusManager.h - Manages InstrumentBus objects for pull-based audio routing
 *
 * The InstrumentBusManager maintains all InstrumentBus objects and provides methods
 * for creating, accessing, and resetting them.
 *
 * Author: Claude Code / RTcmix Development Team
 */

#ifndef _INSTRUMENTBUSMANAGER_H_
#define _INSTRUMENTBUSMANAGER_H_

#include <rt_types.h>
#include <bus.h>
#include <vector>

class InstrumentBus;
class Instrument;
class TaskManager;


class InstrumentBusManager {
public:
    /**
     * Create a new InstrumentBusManager.
     *
     * @param busCount     Maximum number of buses
     * @param bufsamps     Frames per buffer (RTBUFSAMPS)
     */
    InstrumentBusManager(int busCount, int bufsamps);

    ~InstrumentBusManager();

    /**
     * Get or create an InstrumentBus for the specified aux bus.
     *
     * @param busID  The aux bus number
     * @return       Pointer to the InstrumentBus (never NULL)
     */
    InstrumentBus* getOrCreateInstBus(int busID);

    /**
     * Get an InstrumentBus for the specified aux bus (NULL if not yet created).
     *
     * @param busID  The aux bus number
     * @return       Pointer to the InstrumentBus, or NULL if not created
     */
    InstrumentBus* getInstBus(int busID) const;

    /**
     * Check if an InstrumentBus exists for the specified bus.
     *
     * @param busID  The aux bus number
     * @return       true if an InstrumentBus exists
     */
    bool hasInstBus(int busID) const;

    /**
     * Register an instrument as a consumer of an InstrumentBus.
     *
     * @param busID  The aux bus number
     * @param inst   The instrument that reads from this bus
     */
    void addConsumer(int busID, Instrument* inst);

    /**
     * Reset all InstrumentBus objects for a new audio run.
     */
    void reset();

    /**
     * Get the number of InstrumentBus objects currently active.
     */
    int getActiveInstBusCount() const;

#ifdef MULTI_THREAD
    /**
     * Set the TaskManager for all InstrumentBus objects.
     *
     * @param tm  Pointer to the TaskManager
     */
    void setTaskManager(TaskManager* tm);
#endif

private:
    std::vector<InstrumentBus*> mInstBuses;  /* InstrumentBus pointers (indexed by bus ID) */
    int mBufsamps;               /* Frames per buffer */
    int mActiveInstBusCount;     /* Number of active InstrumentBus objects */

#ifdef MULTI_THREAD
    TaskManager* mTaskManager;
#endif

    /* Prevent copying */
    InstrumentBusManager(const InstrumentBusManager&);
    InstrumentBusManager& operator=(const InstrumentBusManager&);
};

#endif /* _INSTRUMENTBUSMANAGER_H_ */
