/* A non-interpolating table lookup (one-shot oscillator) class, by John Gibson
   (This is like table in cmix genlib.)
*/

#if !defined(__TableN_h)
#define __TableN_h

#include "Oscil.h"

class TableN : public Oscil
{
  protected:  
  public:
    TableN(double srate, MY_FLOAT duration, double *aTable, int tableSize);
    ~TableN();
    MY_FLOAT tick(long nsample, MY_FLOAT amp);
};

#endif

