#include "../rtstuff/Instrument.h"

class RTcmix
{

public:
	RTcmix();			// 44.1k/stereo default
	RTcmix(float, int);		// set SR and NCHANS
	RTcmix(float, int, int);	// set SR, NCHANS, BUFSIZE
	Instrument* cmd(char*, int, double, ...); // for numeric params
	Instrument* cmd(char*, int, char*, ...); // for string params
	double cmd(char*); // for commands with no params
	void printOn();
	void printOff();
	void panic();
	void close();
private:
	void init(float, int, int);	// called by all constructors
};

// handy utility function...
/* RTtimeit takes a floating point number of seconds (interval) and a pointer
   to a void-returning function and sets up a timer to call that function
   every interval seconds.  Setting interval to 0.0 should disable the
   timer */
#include <signal.h>
void RTtimeit(float interval, sig_t func);
