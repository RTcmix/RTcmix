#ifndef _RTCMIX_TYPES_H_
#define _RTCMIX_TYPES_H_ 1

#include <stdio.h>

typedef enum {
   VoidType = 0,
   DoubleType,
   StringType,
   HandleType,
   ArrayType
} RTcmixType;

typedef enum {
   PFieldType,
   InstrumentPtrType,
   AudioStreamType
} RTcmixHandleType;

typedef struct _handle {
   RTcmixHandleType type;
   void *ptr;
} *Handle;

typedef struct {
   unsigned int len;    // number of elements in <data> array
   double *data;
} Array;

typedef union {
   double		number;
   const char	*string;
   Handle		handle;
   Array		*array;
} Value;

// Arg operates as a struct in C, and a class in C++.

#ifdef __cplusplus
struct Arg {
#else
typedef struct {
#endif
   RTcmixType type;
   Value val;
#ifdef __cplusplus
   Arg() : type(VoidType) {}
   ~Arg();
   RTcmixType getType() const { return type; }
   void operator = (double d) { type = DoubleType; val.number = d; }
   void operator = (const char *c) { type = StringType; val.string = c; }
   void operator = (const Handle h) { type = HandleType; val.handle = h; }
   void operator = (Array *a) { type = ArrayType; val.array = a; }
   operator double () const { return val.number; }
   operator float () const { return (float) val.number; }
   operator int () const { return (int) val.number; }
   operator const char *() const { return val.string; }
   operator Handle () const { return val.handle; }
   operator Array *() const { return val.array; }
   void printInline(FILE *) const;
};
#else
} Arg;
#endif

#endif /* _RTCMIX_TYPES_H_ */
