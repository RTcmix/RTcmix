//
//  debug.h
//  RTcmix
//
//  Created by Douglas Scott on 12/30/19.
//

#ifndef debug_h
#define debug_h

#undef DEBUG_TRACE
//#define DEBUG_TRACE 1
//#define DEBUG_TRACE 2
//#define DEBUG_MINC_MEMORY
//#define DEBUG_NODE_MEMORY

#include "minc_internal.h"

#if defined(DEBUG_TRACE)

#define MAX_SPACES 128

class Trace {
public:
    Trace(const char *func) : mFunc(func) {
        rtcmix_print("%s%s -->\n", spaces, mFunc);
        ++sTraceDepth;
        const int spaceCount = std::min(MAX_SPACES, sTraceDepth*3);
        for (int n =0; n<spaceCount; ++n) { spaces[n] = ' '; }
        spaces[spaceCount] = '\0';
    }
    static char *getBuf() { return sMsgbuf; }
    static void printBuf() { rtcmix_print("%s%s", spaces, sMsgbuf); }
    ~Trace() {
        --sTraceDepth;
        const int spaceCount = std::min(MAX_SPACES, sTraceDepth*3);
        for (int n =0; n<spaceCount; ++n) { spaces[n] = ' '; }
        spaces[spaceCount] = '\0';
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
