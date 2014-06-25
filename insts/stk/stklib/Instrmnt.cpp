// Instrmnt.C -- hacked version (by BGG) for RTcmix from Perry/Gary's STK
//	some of the Instrmnt functions aren't used in RTcmix, so
//	they haven't been "fixed" to work within RTcmix

// original head/comment:

/***************************************************/
/*! \class Instrmnt
    \brief STK instrument abstract base class.

    This class provides a common interface for
    all STK instruments.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2002.
*/
/***************************************************/

#include "Instrmnt.h"
#include <ugens.h>

Instrmnt :: Instrmnt()
{
}

Instrmnt :: ~Instrmnt()
{
}

void Instrmnt :: setFrequency(MY_FLOAT frequency)
{
  rtcmix_advise("Instrmnt", "virtual setFrequency function call!");
}

MY_FLOAT Instrmnt :: lastOut() const
{
  return lastOutput;
}

MY_FLOAT *Instrmnt :: tick(MY_FLOAT *vector, unsigned int vectorSize)
{
  for (unsigned int i=0; i<vectorSize; i++)
    vector[i] = tick();

  return vector;
}

void Instrmnt :: controlChange(int number, MY_FLOAT value)
{
}
