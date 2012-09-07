#define maxdl 14000

struct delayq
{
	int p,del;
	float d[maxdl],c1,c2;
};

#ifdef __cplusplus

#include <RefCounted.h>
#include <Lockable.h>

class DelayQueue : public delayq, public RefCounted, public Lockable
{
};

#else

typedef struct delayq delayq;

#endif
