class Instrument;
class PFieldSet;

class RTcmix
{

public:
	RTcmix();			// 44.1k/stereo default
	RTcmix(float, int);		// set SR and NCHANS
	RTcmix(float, int, int, const char* opt1=0, const char *opt2=0, const char *opt3=0);	// set SR, NCHANS, BUFSIZE, up to 3 options
	Instrument* cmd(char*, int, double, ...); // for numeric params
	Instrument* cmd(char*, int, char*, ...); // for string params
#ifdef PFIELD_CLASS
	Instrument* cmd(char*, const PFieldSet &); // for PFieldSet
#endif
	double cmd(char*); // for commands with no params
	double cmdval(char*, int, double, ...); // value return (numeric params)
	double cmdval(char*, int, char*, ...); // value return (string params)
	void printOn();
	void printOff();
	void panic();
	void close();
private:
	void init(float, int, int, const char*, const char*, const char*);	// called by all constructors
};

// handy utility function...
/* RTtimeit takes a floating point number of seconds (interval) and a pointer
   to a void-returning function and sets up a timer to call that function
   every interval seconds.  Setting interval to 0.0 should disable the
   timer */
#include <signal.h>
void RTtimeit(float interval, sig_t func);
