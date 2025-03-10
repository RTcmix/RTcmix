/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _RTOPTION_H_
#define _RTOPTION_H_ 1

/* Class for storing all our run-time options.

   The class is static, containing only static members and methods.  To 
   use from C++, refer to the method you want with the class name prefix:

      if (RTOption::print())
         print some stuff

   From C code, you must use the get* and set* functions at the bottom
   of this file.

   You must call RTOption::init() early in main() so that some data members
   will be initialized.

   ---

   To add new options, you need to edit RTOption.cpp/h and set_option.cpp.
   Please add your option after the existing ones of the same type (bool,
   number or string), so that each block of changes has all the options
   in the same order.  This makes it easier to find things.

   1. Add a kOption* define below.  This is the key string that a user will
      write in .rtcmixrc or in the set_option script call.  Please stick to
      the naming conventions already established.

   2. Add static accessor definitions for a bool, number or string option
      below.  Note that (only) the string setter function must be declared
      here and defined in RTOption.cpp (search for "String option setting
      methods").  Numbers can be int or double.

   3. Add a private static member variable to hold the option state, below.

   4. Initialize the static member variable at the top of RTOption.cpp.
      Strings must be initialized instead in the init method, in RTOption.cpp.

   5. Add code to RTOption::readConfigFile (RTOption.cpp) for your option.
      Keep bool, number and string options separate, as they are now.

   6. Add code to RTOption::writeConfigFile (RTOption.cpp) for your option.

   7. Add a line to RTOption::dump() (RTOption.cpp).

   8. If your option would need to be read/written from a C file, add it
      to the appropriate get_* and set_* C functions at the bottom of 
      RTOption.cpp.

   9. Add a symbol to the ParamType enum at the top of set_option.cpp.
      Please keep all items in set_option.cpp in the same order that
      they appear in RTOption.cpp/h, for ease of maintenance.

  10. Below this enum, add an entry to the _param_list array.

  11. In set_option.cpp, write a case statement to match your enum symbol
      in _set_key_value_option, using the _str_to_* functions for string
      conversion.


                                  by John Gibson and Doug Scott, 6/27/04
*/

#include <stdlib.h>
#include <string.h>

#define CONF_FILENAME ".rtcmixrc"
#define DEFAULT_BUFFER_FRAMES 512.0
#ifdef MSPAUDIO
#define DEFAULT_BUFFER_COUNT 1
#else
#define DEFAULT_BUFFER_COUNT 2
#endif
#define DEFAULT_OSC_INPORT 7770
#define DEFAULT_MUTE_THRESHOLD 0.0	/* means no muting */

#define DEVICE_MAX   64
#define MAX_OUTPUT_DEVICES 3

#define DEFAULT_PRINT_LIST_LIMIT 16
#define DEFAULT_PARSER_WARNINGS 0

// Option names.  These are the keys that appear in the .rtcmixrc file.
// They're also the <option_name> used with the get_*_option C functions.

// bool options
#define kOptionAudio            "audio"
#define kOptionPlay             "play"
#define kOptionRecord           "record"
#define kOptionClobber          "clobber"
#define kOptionReportClipping   "report_clipping"
#define kOptionCheckPeaks       "check_peaks"
#define kOptionExitOnError      "exit_on_error"
#define kOptionBailOnError      "bail_on_error"
#define kOptionBailOnParserWarning "bail_on_parser_warning"
#define kOptionAutoLoad         "auto_load"
#define kOptionFastUpdate       "fast_update"
#define kOptionRequireSampleRate	"require_sample_rate"
#define kOptionPrintSuppressUnderbar "print_suppress_underbar"
#define kOptionBailOnUndefinedFunction "bail_on_undefined_function"
#define kOptionSendMIDIRecordAutoStart "send_midi_record_auto_start"

// number options
#define kOptionBufferFrames     "buffer_frames"
#define kOptionBufferCount      "buffer_count"
#define kOptionOSCInPort        "osc_inport"
#define kOptionPrint            "print"
#define kOptionPrintListLimit    "print_list_limit"
#define kOptionParserWarnings   "parser_warnings"
#define kOptionMuteThreshold	"mute_threshold"

// string options
#define kOptionDevice           "device"
#define kOptionInDevice         "indevice"
#define kOptionOutDevice        "outdevice"
#define kOptionOutDevice2       "outdevice2"
#define kOptionOutDevice3       "outdevice3"
#define kOptionMidiInDevice     "midi_indevice"
#define kOptionMidiOutDevice    "midi_outdevice"
#define kOptionOSCHost          "localhost"  // NB: for unimplemented transmit
#define kOptionDSOPath          "dso_path"
#define kOptionRCName           "rcname"
#define kOptionHomeDir          "homedir"
#define kOptionSuppressedFunNames "suppressed_fun_names"


#ifdef __cplusplus

class RTOption {
public:
	RTOption() {};
	~RTOption() {};

	// must call this to initialize string members
	static void init();

	static int readConfigFile(const char *fileName);
	static int writeConfigFile(const char *fileName);

	// bool options

	static bool audio() { return _audio; }
	static bool audio(const bool setIt) { _audio = setIt; return _audio; }

	static bool play() { return _play; }
	static bool play(const bool setIt) { _play = setIt; return _play; }

	static bool record() { return _record; }
	static bool record(const bool setIt) { _record = setIt; return _record; }

	static bool clobber() { return _clobber; }
	static bool clobber(const bool setIt) { _clobber = setIt; return _clobber; }

	static bool reportClipping() { return _reportClipping; }
	static bool reportClipping(const bool setIt) { _reportClipping = setIt;
													return _reportClipping; }

	static bool checkPeaks() { return _checkPeaks; }
	static bool checkPeaks(const bool setIt) { _checkPeaks = setIt;
													return _checkPeaks; }

	static bool exitOnError() { return _exitOnError; }
	static bool exitOnError(const bool setIt) { _exitOnError = setIt;
													return _exitOnError; }
    
    static bool bailOnError() { return _bailOnError; }
    static bool bailOnError(const bool setIt) { _bailOnError = setIt;
        return _bailOnError; }

    static bool bailOnParserWarning() { return _bailOnParserWarning; }
    static bool bailOnParserWarning(const bool setIt) { _bailOnParserWarning = setIt;
        return _bailOnParserWarning; }

	static bool autoLoad() { return _autoLoad; }
	static bool autoLoad(const bool setIt) { _autoLoad = setIt;
													return _autoLoad; }

	static bool fastUpdate() { return _fastUpdate; }
	static bool fastUpdate(const bool setIt) { _fastUpdate = setIt;
													return _fastUpdate; }

	static bool requireSampleRate() { return _requireSampleRate; }
	static bool requireSampleRate(const bool setIt) { _requireSampleRate = setIt;
		return _requireSampleRate; }

    static bool printSuppressUnderbar() { return _printSuppressUnderbar; }
    static bool printSuppressUnderbar(const bool setIt) { _printSuppressUnderbar = setIt;
        return _printSuppressUnderbar; }

    static bool bailOnUndefinedFunction() { return _bailOnUndefinedFunction; }
    static bool bailOnUndefinedFunction(const bool setIt) { _bailOnUndefinedFunction = setIt;
        return _bailOnUndefinedFunction; }

    static bool sendMIDIRecordAutoStart() { return _sendMIDIRecordAutoStart; }
    static bool sendMIDIRecordAutoStart(const bool setIt) { _sendMIDIRecordAutoStart = setIt;
        return _sendMIDIRecordAutoStart; }

	// number options

	static double bufferFrames() { return _bufferFrames; }
	static double bufferFrames(const double frames) { _bufferFrames = frames;
													return _bufferFrames; }

	static int bufferCount() { return _bufferCount; }
	static int bufferCount(int count) { _bufferCount = count;
													return _bufferCount; }

	static int oscInPort() { return _oscInPort; }
	static int oscInPort(int portnum) { _oscInPort = portnum;
													return _oscInPort; }

	static int print() { return _print; }
	static int print(int setIt) { _print = setIt; return _print; }

    static unsigned parserWarnings() { return _parserWarnings; }
    static unsigned parserWarnings(unsigned setIt) { _parserWarnings = setIt; return _parserWarnings; }

    static int printListLimit() { return _printListLimit; }
    static int printListLimit(int setIt) { _printListLimit = setIt; return _printListLimit; }

	static double muteThreshold() { return _muteThreshold; }
	static double muteThreshold(double thresh) { _muteThreshold = thresh; return _muteThreshold; }

	// string options

	// WARNING: If no string as been assigned, do not expect the get method
	// to return NULL!  Instead, check that strlen is > 0.

	static char *device() { return _device; }
	static char *device(const char *devName);

	static char *inDevice() { return _inDevice; }
	static char *inDevice(const char *devName);

	static char *outDevice(int devIndex=0) { return _outDevice[devIndex]; }
	static char *outDevice(const char *devName, int devIndex=0);

	static char *midiInDevice() { return _midiInDevice; }
	static char *midiInDevice(const char *devName);

	static char *midiOutDevice() { return _midiOutDevice; }
	static char *midiOutDevice(const char *devName);

	static char *oscHost() { return _oscHost; }
	static char *oscHost(const char *oscHost);

	static char *dsoPath() { return _dsoPath; }
	static char *dsoPath(const char *pathName);
	static char *dsoPathPrepend(const char *pathName);
	static char *dsoPathAppend(const char *pathName);

	static char *homeDir() { return _homeDir; }

	static char *rcName() { return _rcName; }
	static char *rcName(const char *rcName);

    static char *suppressedFunNamelist() { return _suppressedNamelist; };
    static char *suppressedFunNamelist(const char *nameList);

	static void dump();

private:
	static void reportError(const char *format, const char *msg1, const char *msg2);

	// bool options
	static bool _audio;
	static bool _play;
	static bool _record;
	static bool _clobber;
	static bool _reportClipping;
	static bool _checkPeaks;
	static bool _exitOnError;
    static bool _bailOnError;
    static bool _bailOnParserWarning;
	static bool _autoLoad;
	static bool _fastUpdate;
	static bool _requireSampleRate;
    static bool _printSuppressUnderbar;
    static bool _bailOnUndefinedFunction;
    static bool _sendMIDIRecordAutoStart;

	// number options
	static double _bufferFrames;
	static int _bufferCount;
	static int _oscInPort;
	static int _print;
    static int _printListLimit;
    static unsigned _parserWarnings;
	static double _muteThreshold;

	// string options
	static char _device[];
	static char _inDevice[];
	static char _outDevice[MAX_OUTPUT_DEVICES][DEVICE_MAX];
	static char _midiInDevice[];
	static char _midiOutDevice[];
	static char _oscHost[];
	static char _dsoPath[];
	static char _homeDir[];
	static char _rcName[];
    static char _suppressedNamelist[];
};

extern "C" {
#endif // __cplusplus

// These are for C code that needs to query options.	See the top of this
// file for the supported option_name strings.

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

#endif /* _RTOPTION_H_ */
