#include "RTcmix.h"

class PRTcmix : public RTcmix
{
public:
	PRTcmix();					// 44.1k/stereo default
	PRTcmix(float, int);		// set SR and NCHANS
	PRTcmix(float, int, int);	// set SR, NCHANS, BUFSIZE
	void perlparse(char *);
	double getfval(char *);
};
