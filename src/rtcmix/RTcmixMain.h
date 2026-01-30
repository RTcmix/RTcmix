#ifndef _RTCMIXMAIN_H_
#define _RTCMIXMAIN_H_

#include <RTcmix.h>
#include "DynamicLib.h"
#ifdef OSC
#include <lo/lo.h>
#endif

class RTcmixMain : public RTcmix {
public:
#ifdef EMBEDDED 
	RTcmixMain();  // called from main.cpp
    
	// BGG -- for flushing Queue/Heap frpm flush_sched() (main.cpp)
	void resetQueueHeap();
	// BGG -- experimental dynloading of RTcmix insts in max/msp
	DynamicLib theDSO;
	int doload(char *dsoPath);
	void unload();
#else
	RTcmixMain(int argc, char **argv, char **env);	// called from main.cpp
#endif
    ~RTcmixMain();
	void			run();

protected:
	// Initialization methods.
	void			parseArguments(int argc, char **argv, char **env);
	static void		interrupt_handler(int);
	static void		signal_handler(int);
	static void		set_sig_handlers();

	static void *	sockit(void *);
#ifdef OSC
	// Give access to command_handler()
	friend lo_server_thread start_osc_thread(const char *, int (*)(const char*, int));
	static void		set_osc_port(const char *port);
	static const char *	get_osc_port();
    int             runUsingOSC();
	static void *   OSC_Server(void *);
	static int		command_handler(const char *path, const char *types, lo_arg **argv,
									int argc, lo_message data, void *user_data);
	static int		default_osc_handler(const char *path, const char *types, lo_arg **argv,
									int argc, lo_message data, void *user_data);
	static void		stop_OSC_Server();
#endif
    int             runUsingSockit();
private:
	char *			makeDSOPath(const char *progPath);
	static int 		xargc;	// local copy of arg count
	static char *	xargv[/*MAXARGS + 1*/];
	static char **	xenv;
	static volatile sig_atomic_t interrupt_handler_called;
	static volatile sig_atomic_t signal_handler_called;
	static int		noParse;
	static int		parseOnly;
#ifdef NETAUDIO
	static int		netplay;     // for remote sound network playing
#endif
	/* for more than 1 socket, set by -s flag to CMIX as offset from MYPORT */
	static int		socknew;
#ifdef OSC
	static const char *osc_port;
    static lo_server_thread osc_thread_handle;
	static bool exit_osc;
#endif
};

#endif	// _RTCMIXMAIN_H_
