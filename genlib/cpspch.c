#include <math.h>

float cpspch(float pch)
{
  int oct;
  float retval;
  oct = pch;
  retval = pow(2.,oct+8.333333333*(pch-oct))*1.021975;
  return retval;
}
