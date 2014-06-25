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
    TableN(double srate, double duration, double *aTable, int tableSize);
    ~TableN();
    double tick(long nsample, double amp);
};

#endif

