#include <sys/types.h>
#include <sys/stat.h>

struct SFDESC {
	int sf_chans;
	int sf_class;
	int sf_srate;
};

#define	INT 2		/* sizeof(short) */
#define	FLOAT 4
#define	READ 0

