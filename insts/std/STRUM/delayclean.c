#include <ugens.h>
#include "delayq.h"

void delayclean(delayq *q)
/* clean out (or initialize) delay line for delay, delayset */
{
 int i;

 for (i=0;i<maxdl;i++)
   q->d[i] = 0;
}
