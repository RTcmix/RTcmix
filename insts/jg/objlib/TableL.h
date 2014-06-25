/* An interpolating table lookup (one-shot oscillator) class, by John Gibson
   (This is like tablei in cmix genlib.)
*/

#if !defined(__TableL_h)
#define __TableL_h

#include "TableN.h"

class TableL : public TableN
{
  protected:  
  public:
    TableL(double srate, double duration, double *aTable, int tableSize);
    ~TableL();
    double tick(long nsample, double amp);
};

#endif

