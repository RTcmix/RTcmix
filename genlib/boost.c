#include <math.h>

float boost(float plft)
{
	return(1./sqrt(plft*plft + (1.-plft)*(1.-plft)));
}
