#ifndef _RTCMIX_TYPES_H_
#define _RTCMIX_TYPES_H_ 1

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

struct _handle {
   RTcmixHandleType type;
   void *ptr;
};

typedef struct _handle *Handle;

typedef struct {
   unsigned int len;    // number of elements in <data> array
   double *data;
} Array;

typedef union {
   double number;
   char   *string;
   Handle handle;
   Array  *array;
} Value;

typedef struct {
   RTcmixType type;
   Value val;
} Arg;

#endif /* _RTCMIX_TYPES_H_ */
