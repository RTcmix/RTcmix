#include <math.h>

float octcps(float cps)
{
  float result;
  result = (log(cps/1.021975)/.69314718); 
  return result;
	/* .69etc is log of 2., 1.02etc is offset for middle C */
}
