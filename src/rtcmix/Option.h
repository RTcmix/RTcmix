/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Class for storing all our run-time options.

   The class is static, containing only static members and methods.  To 
   use from C++, refer to the method you want with the class name prefix:

      if (Option::print())
         print some stuff

   From C code, you must use the get* and set* option code at the bottom
   of this file.

   You must call Option::init() early in main() so that some data members
   will be initialized.
                                  by John Gibson and Doug Scott, 6/27/04
*/

#ifndef _OPTION_H_
#define _OPTION_H_ 1

#include <stdlib.h>
#include <string.h>

#define CONF_FILENAME ".rtcmixrc"
#define DEFAULT_BUFFER_FRAMES 4096.0

// Option names.  These are the keys that appear in the .rtcmixrc file.
// They're also the <option_name> used with the get_*_option C functions.

// bool options
#define kOptionAudio            "audio"
#define kOptionPlay             "play"
#define kOptionRecord           "record"
#define kOptionClobber          "clobber"
#define kOptionPrint            "print"
#define kOptionReportClipping   "report_clipping"
#define kOptionCheckPeaks       "check_peaks"

// double options
#define kOptionBufferFrames     "buffer_frames"

// string options
#define kOptionDevice           "device"
#define kOptionInDevice         "indevice"
#define kOptionOutDevice        "outdevice"
#define kOptionDSOPath          "dso_path"
#define kOptionRCName           "rcname"
#define kOptionHomeDir          "homedir"


#ifdef __cplusplus

class Option {
public:
	Option() {};
	~Option() {};

	// must call this to initialize string members
	static void init();

	static int readConfigFile(const char *fileName);

	static bool audio() { return _audio; }
	static bool audio(const bool setIt) { _audio = setIt; return _audio; }

	static bool play() { return _play; }
	static bool play(const bool setIt) { _play = setIt; return _play; }

	static bool record() { return _record; }
	static bool record(const bool setIt) { _record = setIt; return _record; }

	static bool clobber() { return _clobber; }
	static bool clobber(const bool setIt) { _clobber = setIt; return _clobber; }

	static bool print() { return _print; }
	static bool print(const bool setIt) { _print = setIt; return _print; }

	static bool reportClipping() { return _reportClipping; }
	static bool reportClipping(const bool setIt) { _reportClipping = setIt;
													return _reportClipping; }

	static bool checkPeaks() { return _checkPeaks; }
	static bool checkPeaks(const bool setIt) { _checkPeaks = setIt;
													return _checkPeaks; }

	static double bufferFrames() { return _bufferFrames; }
	static double bufferFrames(const double frames) { _bufferFrames = frames;
													return _bufferFrames; }

	static char *device() { return _device; }
	static char *device(const char *devName);

	static char *inDevice() { return _inDevice; }
	static char *inDevice(const char *devName);

	static char *outDevice() { return _outDevice; }
	static char *outDevice(const char *devName);

	static char *dsoPath() { return _dsoPath; }
	static char *dsoPath(const char *pathName);

	static char *homeDir() { return _homeDir; }

	static char *rcName() { return _rcName; }
	static char *rcName(const char *rcName);

	static void dump();

private:
	static bool _audio;
	static bool _play;
	static bool _record;
	static bool _clobber;
	static bool _print;
	static bool _reportClipping;
	static bool _checkPeaks;

	static double _bufferFrames;

	static char _device[];
	static char _inDevice[];
	static char _outDevice[];
	static char _dsoPath[];

	static char _homeDir[];
	static char _rcName[];
};

extern "C" {
#endif // __cplusplus

// These are for C code that needs to query options.	See Option.h for the
// supported option_name strings.

int get_bool_option(const char *option_name);
void set_bool_option(const char *option_name, int value);
double get_double_option(const char *option_name);
void set_double_option(const char *option_name, double value);
char *get_string_option(const char *option_name);
void set_string_option(const char *option_name, const char *value);

void option_dump(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _OPTION_H_ */
