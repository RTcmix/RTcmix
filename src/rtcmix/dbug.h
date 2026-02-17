/* RTcmix debug macros - centralized definitions
 *
 * To enable a specific debug category, change #undef to #define below
 * (or #define it before including this file in a .cpp).
 * Defining ALLBUG enables all categories at once.
 *
 *   ALLBUG - Enables all debug output
 *   BBUG   - Bus debugging (verbose!)
 *   DBUG   - General debug
 *   WBUG   - "Where we are" prints
 *   IBUG   - Instrument and InstrumentBus debugging
 */

#ifndef _DBUG_H_
#define _DBUG_H_

#undef ALLBUG
#undef BBUG
#undef DBUG
#undef WBUG
#undef IBUG

#ifdef ALLBUG
#define BBUG
#define DBUG
#define WBUG
#define IBUG
#endif

#endif /* _DBUG_H_ */
