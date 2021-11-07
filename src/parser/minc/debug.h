//
//  debug.h
//  RTcmix
//
//  Created by Douglas Scott on 12/30/19.
//

#ifndef debug_h
#define debug_h

#undef DEBUG_TRACE
// #define DEBUG_TRACE 2
// #define DEBUG_MEMORY

#include "minc_internal.h"

#if defined(DEBUG_TRACE)

class Trace {
public:
    Trace(const char *func) : mFunc(func) {
        rtcmix_print("%s%s -->\n", spaces, mFunc);
        ++sTraceDepth;
        for (int n =0; n<sTraceDepth*3; ++n) { spaces[n] = ' '; }
        spaces[sTraceDepth*3] = '\0';
    }
    static char *getBuf() { return sMsgbuf; }
    static void printBuf() { rtcmix_print("%s%s", spaces, sMsgbuf); }
    ~Trace() {
        --sTraceDepth;
        for (int n =0; n<sTraceDepth*3; ++n) { spaces[n] = ' '; }
        spaces[sTraceDepth*3] = '\0';
        rtcmix_print("%s<-- %s\n", spaces, mFunc);
    }
private:
    const char *mFunc;
    static char sMsgbuf[];
    static int sTraceDepth;
    static char spaces[];
};

#if DEBUG_TRACE==2
#ifdef __GNUC__
#define ENTER() Trace __trace__(__PRETTY_FUNCTION__)
#else
#define ENTER() Trace __trace__(__FUNCTION__)
#endif
#else
#define ENTER()
#endif

#define TPRINT(...) do { snprintf(Trace::getBuf(), 256, __VA_ARGS__); Trace::printBuf(); } while(0)
#else
#define ENTER()
#define TPRINT(...)

#endif  /* defined(DEBUG_TRACE) */

#ifdef DEBUG_MEMORY
#define MPRINT(...) rtcmix_print(__VA_ARGS__)
#else
#define MPRINT(...)
#endif

#endif /* debug_h */
