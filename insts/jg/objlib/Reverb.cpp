/* Reverb Abstract Class, by Tim Stilson, 1998
   Integrated into STK by Gary Scavone with T60 argument.
   Hacked at by JG.
*/

#include "Reverb.h"


Reverb :: Reverb()
{
}


Reverb :: ~Reverb()
{
}


void Reverb :: setEffectMix(MY_FLOAT mix)
{
   fprintf(stderr, "WARNING! Reverb class setEffectMix called!\n");
}


MY_FLOAT Reverb :: lastOutput()
{
   fprintf(stderr, "WARNING! Reverb class lastOutput called!\n");
   return 0.0;
}


MY_FLOAT Reverb :: lastOutputL()
{
   fprintf(stderr, "WARNING! Reverb class lastOutputL called!\n");
   return 0.0;
}


MY_FLOAT Reverb :: lastOutputR()
{
   fprintf(stderr, "WARNING! Reverb class lastOutputR called!\n");
   return 0.0;
}


MY_FLOAT Reverb :: tick(MY_FLOAT input)
{
   fprintf(stderr, "WARNING! Reverb class tick called!\n");
   return 0.0;
}


int Reverb :: isprime(int val)
{
   if (val == 2)
      return 1;
   if (val & 1) {
      for (int i = 3; i < (int)sqrt((double)val) + 1; i += 2)
         if ((val % i) == 0)
            return 0;
      return 1;           // prime
   }
   else
      return 0;           // even
}


