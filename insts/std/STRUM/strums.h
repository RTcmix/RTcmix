/* For plucked string instruments using 'strum' */
#define maxlen 2000
struct strumq
{
	int n,p;
	float d[maxlen],a[4],dcz1,dcb1,dca1,dca0;
};

#ifdef __cplusplus

#include <RefCounted.h>
#include <Lockable.h>

class StrumQueue : public strumq, public RefCounted, public Lockable
{
} ;

#else

typedef struct strumq strumq;

#endif
