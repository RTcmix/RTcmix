#if !defined(__DCBlock_h)
#define __DCBlock_h

#include "JGFilter.h"

class DCBlock : public JGFilter
{
  public:
    DCBlock();
    ~DCBlock();
    void clear();
    double tick(double sample);
};

#endif
