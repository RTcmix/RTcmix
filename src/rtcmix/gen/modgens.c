/* RTcmix  - Copyright (C) 2002  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <ugens.h>


/* -------------------------------------------------------------- addgens --- */
double
m_addgens(float p[], int n_args, double pp[])
{
   int   destslot, srcslot1, srcslot2, normalize, size;

   destslot = (int) p[0];
   srcslot1 = (int) p[1];
   srcslot2 = (int) p[2];
   normalize = (int) p[3];

   size = combine_gens(destslot, srcslot1, srcslot2, normalize,
                                                ADD_GENS, "addgens");
   return (double) size;
}


/* ------------------------------------------------------------- multgens --- */
double
m_multgens(float p[], int n_args, double pp[])
{
   int   destslot, srcslot1, srcslot2, normalize, size;

   destslot = (int) p[0];
   srcslot1 = (int) p[1];
   srcslot2 = (int) p[2];
   normalize = (int) p[3];

   size = combine_gens(destslot, srcslot1, srcslot2, normalize,
                                                MULT_GENS, "multgens");
   return (double) size;
}


