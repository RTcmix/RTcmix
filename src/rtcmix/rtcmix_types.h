#ifndef _RTCMIX_TYPES_H_
#define _RTCMIX_TYPES_H_ 1

typedef enum {
   VoidType = 0,
   FloatType,
   StringType,
   HandleType,
   ArrayType
} RTcmixType;

// XXX Needs to be double; otherwise, legacy func calls don't work
// MincFloat (Minc/minc_internal.h) must match this type!
//typedef float Float;
typedef double Float;

typedef char *String;

typedef enum {
   PFieldType,
   InstrumentPtrType,
   AudioStreamType
} RTcmixHandleType;

struct _handle {
   RTcmixHandleType type;
   void *ptr;
};

typedef struct _handle *Handle;

typedef struct {
   unsigned int len;    // number of Float's in <data> array
   Float *data;
} Array;

typedef union {
   Float  number;
   String string;
   Handle handle;
   Array  *array;
} Value;

typedef struct {
   RTcmixType type;
   Value val;
} Arg;

#endif /* _RTCMIX_TYPES_H_ */
