

class SENDOSC : public Instrument {

public:
	SENDOSC();
	virtual ~SENDOSC();
	virtual int init(double*, int);
	virtual int run();

	#define MAXOSCMSG 1024
	char oscmessage[MAXOSCMSG];
	char oscmessage2[MAXOSCMSG];
	float oscvals[1024]; // from RTcmix/src/include/maxdispargs.h
	
        char clnt_name[MAXOSCMSG];
        char osc_path[MAXOSCMSG];
        char format[MAXOSCMSG];
        
        int int_message;
        float flt_message;
        char str_message[MAXOSCMSG];

        };
