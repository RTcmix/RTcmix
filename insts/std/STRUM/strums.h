/* For plucked string instruments using 'strum' */
#define maxlen 2000
typedef struct {
        int n,p;
        float d[maxlen],a[4],dcz1,dcb1,dca1,dca0;
        } strumq;

