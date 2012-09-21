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

/* This should probably go someplace else in this file? */
typedef enum {
  NO = 0,
  YES
} Bool;

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

#ifdef MULTI_THREAD
#ifdef __cplusplus

#ifdef MACOSX

#include <libkern/OSAtomic.h>

class AtomicInt
{
    int32_t val;
public:
    AtomicInt(int inVal=0) : val(inVal) {}
    operator int () const { return val; }
    int operator ++ () { return OSAtomicIncrement32(&val); }
    int operator -- () { return OSAtomicDecrement32(&val); }
    int operator = (int rhs) { return (val = rhs); }
    
};

#elif defined(LINUX)

#include <alsa/iatomic.h>

class AtomicInt
{
    atomic_t val;
public:
    AtomicInt(int inVal=0) { atomic_set(&val, inVal); }
    operator int () const { return atomic_read(&val); }
    int operator ++ () { return atomic_inc_return(&val); }
    int operator -- () { return atomic_dec_return(&val); }
    int operator = (int rhs) { atomic_set(&val, rhs); return atomic_read(&val); }
};

#else

typedef int AtomicInt;

#endif

#else

typedef int AtomicInt;

#endif  // __cplusplus

#else	// not MULTI_THREAD

typedef int AtomicInt;

#endif  // MULTI_THREAD

#endif	// _RT_TYPES_H_
