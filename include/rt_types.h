/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef _RT_TYPES_H_
#define _RT_TYPES_H_

/* type of buffer used for internal buses */
#define BUFTYPE float           /* could be double some day */
typedef BUFTYPE *BufPtr;

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


#endif	// _RT_TYPES_H_
