#ifdef SYS5
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#define index(A,B) strchr(A,B)
#else
#include <strings.h>
#endif
