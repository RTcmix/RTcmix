/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Class for storing all our run-time options.  -JGG, 6/27/04 */

#include <assert.h>
#include "Option.h"
#include "conf/Config.h"
#include <ugens.h>         // FIXME: pull all this in just for warn()?


Option::Option()
	: _audioOn(true), _playOn(true), _recordOn(false), _clobberOn(false),
	_printOn(true), _reportClippingOn(true), _checkPeaksOn(true),
	_bufferFrames(DEFAULT_BUFFER_FRAMES),
	_device(NULL), _inDevice(NULL), _outDevice(NULL),
	_dsoPath(NULL), _homeDir(NULL), _rcName(NULL)
{
	// initialize home directory and full path of user's configuration file
	char *dir = getenv("HOME");
	if (dir == NULL)
		return;
	if (strlen(dir) > 256)
		return;
	char *rc = new char[strlen(dir) + 1 + strlen(CONF_FILENAME) + 1];
	strcpy(rc, dir);
	strcat(rc, "/");
	strcat(rc, CONF_FILENAME);
	_homeDir = dir;
	_rcName = rc;
}

Option::~Option()
{
	delete _device;
	delete _inDevice;
	delete _outDevice;
	delete _dsoPath;
	delete _rcName;
}

/* Read configuration file <fileName>, which is probably the full path name
   of the user's .rtcmixrc file, and copy the settings in the file to the
   Option object.  Return 0 if successful, or if the file doesn't exist.
   Return -1 if there is a problem reading the file (such as insufficient
   privileges).
*/
int Option::readConfigFile(const char *fileName)
{
	char *key;

	Config *conf = new Config();
	ConfigErrorCode result = conf->parseFile(fileName);
	if (result == kConfigFileMissingErr) {
		delete conf;
		return -1;			// user doesn't have an rc file; fail silently
	}
	if (result != kConfigNoErr) {
		warn(NULL, "%s \"%s\"", conf->getLastErrorText(), fileName);
		delete conf;
		return -1;
	}

	// bool options ...........................................................

	bool bval;

	key = AUDIO_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		audio(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = PLAY_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		play(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = RECORD_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		record(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = CLOBBER_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		clobber(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = PRINT_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		print(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = REPORT_CLIPPING_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		reportClipping(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = CHECK_PEAKS_STR;
	result = conf->getValue(key, bval);
	if (result == kConfigNoErr)
		checkPeaks(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	// double options .........................................................

	double dval;

	key = BUFFER_FRAMES_STR;
	result = conf->getValue(key, dval);
	if (result == kConfigNoErr)
		bufferFrames(dval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	// string options .........................................................

	char *sval;

	key = DEVICE_STR;
	result = conf->getValue(key, sval);
	if (result == kConfigNoErr)
		device(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = INDEVICE_STR;
	result = conf->getValue(key, sval);
	if (result == kConfigNoErr)
		inDevice(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = OUTDEVICE_STR;
	result = conf->getValue(key, sval);
	if (result == kConfigNoErr)
		outDevice(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	key = DSO_PATH_STR;
	result = conf->getValue(key, sval);
	if (result == kConfigNoErr)
		dsoPath(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf->getLastErrorText(), key);

	delete conf;
	return 0;
}


// String option setting methods

char *Option::device(const char *devName)
{
	delete _device;
	_device = _strdup(devName);
	return _device;
}

char *Option::inDevice(const char *devName)
{
	delete _inDevice;
	_inDevice = _strdup(devName);
	return _inDevice;
}

char *Option::outDevice(const char *devName)
{
	delete _outDevice;
	_outDevice = _strdup(devName);
	return _outDevice;
}

char *Option::dsoPath(const char *pathName)
{
	delete _dsoPath;
	_dsoPath = _strdup(pathName);
	return _dsoPath;
}

char *Option::rcName(const char *rcName)
{
	delete _rcName;
	_rcName = _strdup(rcName);
	return _rcName;
}

// version of strdup that new's its memory
char *Option::_strdup(const char *str)
{
	char *dest = new char[strlen(str) + 1];
	strcpy(dest, str);
	return dest;
}


// ----------------------------------------------------------------------------
// These functions are for C code that needs to query options.

extern Option options;	// FIXME: declared in globals.h

int get_bool_option(const char *option_name)
{
	if (!strcmp(option_name, PRINT_STR))
		return (int) options.print();
	else if (!strcmp(option_name, REPORT_CLIPPING_STR))
		return (int) options.reportClipping();
	else if (!strcmp(option_name, CHECK_PEAKS_STR))
		return (int) options.checkPeaks();
	else if (!strcmp(option_name, CLOBBER_STR))
		return (int) options.clobber();
	else if (!strcmp(option_name, AUDIO_STR))
		return (int) options.audio();
	else if (!strcmp(option_name, PLAY_STR))
		return (int) options.play();
	else if (!strcmp(option_name, RECORD_STR))
		return (int) options.record();

	assert(0 && "unsupported option name");		// program error
	return 0;
}

double get_double_option(const char *option_name)
{
	if (!strcmp(option_name, BUFFER_FRAMES_STR))
		return options.bufferFrames();

	assert(0 && "unsupported option name");
	return 0;
}

char *get_string_option(const char *option_name)
{
	if (!strcmp(option_name, DEVICE_STR))
		return options.device();
	else if (!strcmp(option_name, INDEVICE_STR))
		return options.inDevice();
	else if (!strcmp(option_name, OUTDEVICE_STR))
		return options.outDevice();
	else if (!strcmp(option_name, DSO_PATH_STR))
		return options.dsoPath();

	assert(0 && "unsupported option name");
	return 0;
}

