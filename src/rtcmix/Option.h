/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Class for storing all our run-time options.  -JGG, 6/27/04 */

#ifndef _OPTION_H_
#define _OPTION_H_ 1

#include <stdlib.h>
#include <string.h>

#define CONF_FILENAME ".rtcmixrc"
#define DEFAULT_BUFFER_FRAMES 4096

// Option names.  These are the keys that appear in the .rtcmixrc file.
// They're also the <option_name> used with the get_*_option C functions.

// bool options
#define AUDIO_STR					"audio"
#define PLAY_STR					"play"
#define RECORD_STR				"record"
#define CLOBBER_STR				"clobber"
#define PRINT_STR					"print"
#define REPORT_CLIPPING_STR	"report_clipping"
#define CHECK_PEAKS_STR			"check_peaks"

// double options
#define BUFFER_FRAMES_STR		"buffer_frames"

// string options
#define DEVICE_STR				"device"
#define INDEVICE_STR				"indevice"
#define OUTDEVICE_STR			"outdevice"
#define DSO_PATH_STR				"dso_path"


#ifdef __cplusplus

class Option {
public:
	Option();
	~Option();

	int readConfigFile(const char *fileName);

	bool audio() const { return _audioOn; }
	bool audio(const bool setIt) { _audioOn = setIt; return _audioOn; }

	bool play() const { return _playOn; }
	bool play(const bool setIt) { _playOn = setIt; return _playOn; }

	bool record() const { return _recordOn; }
	bool record(const bool setIt) { _recordOn = setIt; return _recordOn; }

	bool clobber() const { return _clobberOn; }
	bool clobber(const bool setIt) { _clobberOn = setIt; return _clobberOn; }

	bool print() const { return _printOn; }
	bool print(const bool setIt) { _printOn = setIt; return _printOn; }

	bool reportClipping() const { return _reportClippingOn; }
	bool reportClipping(const bool setIt) { _reportClippingOn = setIt; return _reportClippingOn; }

	bool checkPeaks() const { return _checkPeaksOn; }
	bool checkPeaks(const bool setIt) { _checkPeaksOn = setIt; return _checkPeaksOn; }

	double bufferFrames() const { return _bufferFrames; }
	double bufferFrames(const double frames) { _bufferFrames = frames; return _bufferFrames; }

	char *device() const { return _device; }
	char *device(const char *devName);

	char *inDevice() const { return _inDevice; }
	char *inDevice(const char *devName);

	char *outDevice() const { return _outDevice; }
	char *outDevice(const char *devName);

	char *dsoPath() const { return _dsoPath; }
	char *dsoPath(const char *pathName);

	char *homeDir() const { return _homeDir; }

	char *rcName() const { return _rcName; }
	char *rcName(const char *rcName);

private:
	char *_strdup(const char *str);

	bool _audioOn;
	bool _playOn;
	bool _recordOn;
	bool _clobberOn;
	bool _printOn;
	bool _reportClippingOn;
	bool _checkPeaksOn;

	double _bufferFrames;

	char *_device;
	char *_inDevice;
	char *_outDevice;
	char *_dsoPath;

	char *_homeDir;
	char *_rcName;
};

extern "C" {
#endif // __cplusplus

// These are for C code that needs to query options.	See Option.h for the
// supported option_name strings.

int get_bool_option(const char *option_name);
double get_double_option(const char *option_name);
char *get_string_option(const char *option_name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _OPTION_H_ */
