#if !defined(__DCBlock_h)
#define __DCBlock_h

#include "Filter.h"

class DCBlock : public Filter
{
  public:
    DCBlock();
    ~DCBlock();
    void clear();
    double tick(double sample);
};

#endif
