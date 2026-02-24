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
#include <map>

class InstrumentBus;
class Instrument;


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
     * Register an instrument as a consumer of an InstrumentBus.
     *
     * @param busID  The aux bus number
     * @param inst   The instrument that reads from this bus
     */
    void addConsumer(int busID, Instrument* inst);

    /**
     * Remove an instrument as a consumer from all InstrumentBus objects.
     * Called from the Instrument destructor.
     *
     * @param inst   The instrument being destroyed
     */
    void removeConsumer(Instrument* inst);

    /**
     * Advance production counters on all active InstrumentBus objects.
     * Called from intraverse after completing a phased TO_AUX or AUX_TO_AUX
     * cycle (after waitForTasks + mixToBus, before the next phase).
     *
     * @param frames          Number of frames produced (typically RTBUFSAMPS)
     * @param currentBufStart The current bufStartSamp (identifies the cycle)
     */
    void advanceAllProduction(int frames, FRAMETYPE currentBufStart);

    /**
     * Reset all InstrumentBus objects for a new audio run.
     */
    void reset();

    /**
     * Get the number of InstrumentBus objects currently active.
     */
    int getActiveInstBusCount() const { return (int)mInstBuses.size(); }

private:
    std::map<int, InstrumentBus*> mInstBuses;  /* busID -> InstrumentBus */
    int mBufsamps;               /* Frames per buffer */

    /* Prevent copying */
    InstrumentBusManager(const InstrumentBusManager&);
    InstrumentBusManager& operator=(const InstrumentBusManager&);
};

#endif /* _INSTRUMENTBUSMANAGER_H_ */
