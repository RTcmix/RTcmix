/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef _RT_TYPES_H_
#define _RT_TYPES_H_

/* type of buffer used for internal buses */
#define BUFTYPE float           /* could be double some day */
typedef BUFTYPE *BufPtr;

#if BUFTYPE == float
#define ZERO 0.0f
#else
#define ZERO 0.0
#endif

/* type for frame counts (Inst start and end points) */
#define FRAMETYPE long long

/* This should probably go someplace else in this file? */
#ifndef NO
typedef enum {
  NO = 0,
  YES
} Bool;
#else
typedef Boolean Bool;
#endif

#ifndef PI
#define      PI     3.141592654
#endif
#ifndef PI2
#define      PI2    6.2831853
#endif

#define FLOAT (sizeof(float))   /* nbytes in floating point word*/
#define INT   (sizeof(int))   /* nbytes in integer word */
#define SHORT (sizeof(short))
#define LONG  (sizeof(long))

#ifdef __cplusplus

#if defined(__cpp_lib_atomic) || __cplusplus >= 199711L
#define USE_ATOMIC 1
#include <atomic>
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#define USE_ATOMIC 1
#include <atomic>
#else
#define USE_ATOMIC 0
#endif

#ifdef MULTI_THREAD

#ifdef MAXMSP
#error Multi-threaded support not yet available for this configuration
#endif

#if USE_ATOMIC

class AtomicInt : private std::atomic<int32_t>
{
public:
    explicit AtomicInt(int inVal=0)  { store(inVal); }
    operator int () const { return load(); };
    void increment() { ++(*this); }
    bool incrementAndTest() { return (++(*this)) == 0; }
    bool decrementAndTest() { return (--(*this)) == 0; }
    int32_t operator = (int rhs) { store(rhs); return load(); }
};

#elif defined(MACOSX)

#include <libkern/OSAtomic.h>

class AtomicInt
{
    int32_t val;
public:
    AtomicInt(int inVal=0) : val(inVal) {}
    operator int () const { return val; }
	void increment() { OSAtomicIncrement32(&val); }
    bool incrementAndTest() { return OSAtomicIncrement32(&val) == 0; }
    bool decrementAndTest() { return OSAtomicDecrement32(&val) == 0; }
    int operator = (int rhs) { return (val = rhs); }
};

#else

#error Tell Doug Scott you are want to compile MULTI_THREAD for this platform.
typedef int AtomicInt;

#endif  // USE_ATOMIC

#else	// !MULTI_THREAD

class AtomicInt
{
    int val;
public:
    AtomicInt(int inVal=0) : val(inVal) {}
    operator int () const { return val; }
    void increment() { ++val; }
    bool incrementAndTest() { return ++val == 0; }
    bool decrementAndTest() { return --val == 0; }
    int operator = (int rhs) { return (val = rhs); }
};

#endif  // !MULTI_THREAD

#endif  // __cplusplus

#endif	// _RT_TYPES_H_
