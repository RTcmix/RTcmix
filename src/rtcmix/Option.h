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
#define DEFAULT_BUFFER_COUNT 2

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
#define kOptionExitOnError      "exit_on_error"
#define kOptionAutoLoad         "auto_load"

// double options
#define kOptionBufferFrames     "buffer_frames"
#define kOptionBufferCount     "buffer_count"

// string options
#define kOptionDevice           "device"
#define kOptionInDevice         "indevice"
#define kOptionOutDevice        "outdevice"
#define kOptionMidiInDevice     "midi_indevice"
#define kOptionMidiOutDevice    "midi_outdevice"
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
	static int writeConfigFile(const char *fileName);

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

	static bool exitOnError() { return _exitOnError; }
	static bool exitOnError(const bool setIt) { _exitOnError = setIt;
													return _exitOnError; }

	static double bufferFrames() { return _bufferFrames; }
	static double bufferFrames(const double frames) { _bufferFrames = frames;
													return _bufferFrames; }

	static int bufferCount() { return _bufferCount; }
	static int bufferCount(int count) { _bufferCount = count;
													return _bufferCount; }
	static bool autoLoad() { return _autoLoad; }
	static bool autoLoad(const bool setIt) { _autoLoad = setIt;
													return _autoLoad; }

	// WARNING: If no string as been assigned, do not expect the get method
	// to return NULL!  Instead, check that strlen is > 0.

	static char *device() { return _device; }
	static char *device(const char *devName);

	static char *inDevice() { return _inDevice; }
	static char *inDevice(const char *devName);

	static char *outDevice() { return _outDevice; }
	static char *outDevice(const char *devName);

	static char *midiInDevice() { return _midiInDevice; }
	static char *midiInDevice(const char *devName);

	static char *midiOutDevice() { return _midiOutDevice; }
	static char *midiOutDevice(const char *devName);

	static char *dsoPath() { return _dsoPath; }
	static char *dsoPath(const char *pathName);
	static char *dsoPathPrepend(const char *pathName);
	static char *dsoPathAppend(const char *pathName);

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
	static bool _exitOnError;
	static bool _autoLoad;

	static double _bufferFrames;
	static int _bufferCount;

	static char _device[];
	static char _inDevice[];
	static char _outDevice[];
	static char _midiInDevice[];
	static char _midiOutDevice[];
	static char _dsoPath[];

	static char _homeDir[];
	static char _rcName[];
};

extern "C" {
#endif // __cplusplus

// These are for C code that needs to query options.	See Option.h for the
// supported option_name strings.

int get_print_option();
char *get_dsopath_option();
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
