/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Class for storing all our run-time options,
	by John Gibson and Doug Scott, 6/27/04.
*/

#include <assert.h>
#include <limits.h>        // PATH_MAX
#include "Option.h"
#include "conf/Config.h"
#include <ugens.h>         // FIXME: pull all this in just for warn()?
#include <iostream>

#define DEVICE_MAX   64
#define DSOPATH_MAX  PATH_MAX * 2

bool Option::_audio = true;
bool Option::_play = true;
bool Option::_record = false;
bool Option::_clobber = false;
bool Option::_print = true;
bool Option::_reportClipping = true;
bool Option::_checkPeaks = true;

double Option::_bufferFrames = DEFAULT_BUFFER_FRAMES;

char Option::_device[DEVICE_MAX];
char Option::_inDevice[DEVICE_MAX];
char Option::_outDevice[DEVICE_MAX];
char Option::_dsoPath[DSOPATH_MAX];
char Option::_homeDir[PATH_MAX];
char Option::_rcName[PATH_MAX];


void Option::init()
{
   _device[0] = 0;
   _inDevice[0] = 0;
   _outDevice[0] = 0;
   _dsoPath[0] = 0;
   _homeDir[0] = 0;
   _rcName[0] = 0;

	// initialize home directory and full path of user's configuration file

	char *dir = getenv("HOME");
	if (dir == NULL)
		return;
	if (strlen(dir) > 256)     // legit HOME not likely to be longer
		return;
	strncpy(_homeDir, dir, PATH_MAX);
	_homeDir[PATH_MAX - 1] = 0;

	char *rc = new char[strlen(dir) + 1 + strlen(CONF_FILENAME) + 1];
	strcpy(rc, dir);
	strcat(rc, "/");
	strcat(rc, CONF_FILENAME);
	strncpy(_rcName, rc, PATH_MAX);
	_rcName[PATH_MAX - 1] = 0;
	delete [] rc;
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
	Config conf;

	ConfigErrorCode result = conf.parseFile(fileName);
	if (result == kConfigFileMissingErr)
		return -1;			// user doesn't have an rc file; fail silently
	if (result != kConfigNoErr) {
		warn(NULL, "%s \"%s\"", conf.getLastErrorText(), fileName);
		return -1;
	}

	// bool options ...........................................................

	bool bval;

	key = kOptionAudio;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		audio(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionPlay;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		play(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionRecord;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		record(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionClobber;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		clobber(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionPrint;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		print(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionReportClipping;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		reportClipping(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionCheckPeaks;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		checkPeaks(bval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	// double options .........................................................

	double dval;

	key = kOptionBufferFrames;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		bufferFrames(dval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	// string options .........................................................

	char *sval;

	key = kOptionDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		device(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionInDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		inDevice(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionOutDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		outDevice(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	key = kOptionDSOPath;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		dsoPath(sval);
	else if (result != kConfigNoValueForKey)
		warn(NULL, "%s: %s.\n", conf.getLastErrorText(), key);

	return 0;
}

// String option setting methods

char *Option::device(const char *devName)
{
	strncpy(_device, devName, DEVICE_MAX);
	_device[DEVICE_MAX - 1] = 0;
	return _device;
}

char *Option::inDevice(const char *devName)
{
	strncpy(_inDevice, devName, DEVICE_MAX);
	_inDevice[DEVICE_MAX - 1] = 0;
	return _inDevice;
}

char *Option::outDevice(const char *devName)
{
	strncpy(_outDevice, devName, DEVICE_MAX);
	_outDevice[DEVICE_MAX - 1] = 0;
	return _outDevice;
}

char *Option::dsoPath(const char *pathName)
{
	strncpy(_dsoPath, pathName, DSOPATH_MAX);
	_dsoPath[DSOPATH_MAX - 1] = 0;
	return _dsoPath;
}

char *Option::rcName(const char *rcName)
{
	strncpy(_rcName, rcName, PATH_MAX);
	_rcName[PATH_MAX - 1] = 0;
	return _rcName;
}

void Option::dump()
{
	cout << kOptionAudio << ": " << _audio << endl;
	cout << kOptionPlay << ": " << _play << endl;
	cout << kOptionRecord << ": " << _record << endl;
	cout << kOptionClobber << ": " << _clobber << endl;
	cout << kOptionPrint << ": " << _print << endl;
	cout << kOptionReportClipping << ": " << _reportClipping << endl;
	cout << kOptionCheckPeaks << ": " << _checkPeaks << endl;
	cout << kOptionBufferFrames << ": " << _bufferFrames << endl;
	cout << kOptionDevice << ": " << _device << endl;
	cout << kOptionInDevice << ": " << _inDevice << endl;
	cout << kOptionOutDevice << ": " << _outDevice << endl;
	cout << kOptionDSOPath << ": " << _dsoPath << endl;
	cout << kOptionHomeDir << ": " << _homeDir << endl;
	cout << kOptionRCName << ": " << _rcName << endl;
}


// ----------------------------------------------------------------------------
// These functions are for C code that needs to query options.

int get_bool_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionPrint))
		return (int) Option::print();
	else if (!strcmp(option_name, kOptionReportClipping))
		return (int) Option::reportClipping();
	else if (!strcmp(option_name, kOptionCheckPeaks))
		return (int) Option::checkPeaks();
	else if (!strcmp(option_name, kOptionClobber))
		return (int) Option::clobber();
	else if (!strcmp(option_name, kOptionAudio))
		return (int) Option::audio();
	else if (!strcmp(option_name, kOptionPlay))
		return (int) Option::play();
	else if (!strcmp(option_name, kOptionRecord))
		return (int) Option::record();

	assert(0 && "unsupported option name");		// program error
	return 0;
}

void set_bool_option(const char *option_name, int value)
{
	if (!strcmp(option_name, kOptionPrint))
		Option::print((bool) value);
	else if (!strcmp(option_name, kOptionReportClipping))
		Option::reportClipping((bool) value);
	else if (!strcmp(option_name, kOptionCheckPeaks))
		Option::checkPeaks((bool) value);
	else if (!strcmp(option_name, kOptionClobber))
		Option::clobber((bool) value);
	else if (!strcmp(option_name, kOptionAudio))
		Option::audio((bool) value);
	else if (!strcmp(option_name, kOptionPlay))
		Option::play((bool) value);
	else if (!strcmp(option_name, kOptionRecord))
		Option::record((bool) value);
	else
		assert(0 && "unsupported option name");
}

double get_double_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionBufferFrames))
		return Option::bufferFrames();

	assert(0 && "unsupported option name");
	return 0;
}

void set_double_option(const char *option_name, double value)
{
	if (!strcmp(option_name, kOptionBufferFrames))
		Option::bufferFrames(value);
	else
		assert(0 && "unsupported option name");
}

char *get_string_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionDevice))
		return Option::device();
	else if (!strcmp(option_name, kOptionInDevice))
		return Option::inDevice();
	else if (!strcmp(option_name, kOptionOutDevice))
		return Option::outDevice();
	else if (!strcmp(option_name, kOptionDSOPath))
		return Option::dsoPath();

	assert(0 && "unsupported option name");
	return 0;
}

void set_string_option(const char *option_name, const char *value)
{
	if (!strcmp(option_name, kOptionDevice))
		Option::device(value);
	else if (!strcmp(option_name, kOptionInDevice))
		Option::inDevice(value);
	else if (!strcmp(option_name, kOptionOutDevice))
		Option::outDevice(value);
	else if (!strcmp(option_name, kOptionDSOPath))
		Option::dsoPath(value);
	else
		assert(0 && "unsupported option name");
}

// This is so we can call dump from within GDB.
void option_dump()
{
	Option::dump();
}

