/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <sndlibsupport.h>
#include "../H/sfheader.h"

/* These stay here (instead of moving to globals.h), because some utility
   programs need them.
*/
int swap;
short isNext;


/* Assuming rheader called before this to fill in <sfh> and check it, 
   all we need to do here is set the <swap> flag.
*/
int
check_byte_order(SFHEADER *sfh, char *prog, char *sfname)
{
#ifdef SNDLIB_LITTLE_ENDIAN
   swap = (sfdataformat(sfh) == snd_16_linear
        || sfdataformat(sfh) == snd_32_float);
#else
   swap = (sfdataformat(sfh) == snd_16_linear_little_endian
        || sfdataformat(sfh) == snd_32_float_little_endian);
#endif
   isNext = 0;

   return 0;
}


