/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Class for storing all our run-time options,
   by John Gibson and Doug Scott, 6/27/04.
*/

#include <assert.h>
#include <limits.h>        // PATH_MAX
#include "RTOption.h"
#include <Config.h>
#include <string.h>
#include "ugens.h"
#ifndef EMBEDDED
#include <iostream>
using namespace std;
#endif

#define DSOPATH_MAX  PATH_MAX * 2
#define OSCHOST_MAX  128
#define SUPPRESSED_NAMELIST_MAX 32768

bool RTOption::_audio = true;
bool RTOption::_play = true;
bool RTOption::_record = false;
bool RTOption::_clobber = false;
bool RTOption::_reportClipping = true;
bool RTOption::_checkPeaks = true;
bool RTOption::_exitOnError = false;	// we override this in main.cpp for standalone
bool RTOption::_bailOnError = false;
bool RTOption::_bailOnParserWarning = false;
bool RTOption::_autoLoad = false;
bool RTOption::_fastUpdate = false;
bool RTOption::_requireSampleRate = true;
bool RTOption::_printSuppressUnderbar = false;
bool RTOption::_bailOnUndefinedFunction = false;
bool RTOption::_sendMIDIRecordAutoStart = false;

double RTOption::_bufferFrames = DEFAULT_BUFFER_FRAMES;
int RTOption::_bufferCount = DEFAULT_BUFFER_COUNT;
int RTOption::_oscInPort = DEFAULT_OSC_INPORT;
double RTOption::_muteThreshold = DEFAULT_MUTE_THRESHOLD;

// BGG see ugens.h for levels
#ifdef EMBEDDED
int RTOption::_print = MMP_RTERRORS; // basic level for max/msp
#else
int RTOption::_print = MMP_PRINTALL; // default print everthing for regular RTcmix
#endif

int RTOption::_printListLimit = DEFAULT_PRINT_LIST_LIMIT;
unsigned RTOption::_parserWarnings = DEFAULT_PARSER_WARNINGS;

char RTOption::_device[DEVICE_MAX];
char RTOption::_inDevice[DEVICE_MAX];
char RTOption::_outDevice[MAX_OUTPUT_DEVICES][DEVICE_MAX];
char RTOption::_midiInDevice[DEVICE_MAX];
char RTOption::_midiOutDevice[DEVICE_MAX];
char RTOption::_oscHost[OSCHOST_MAX];
char RTOption::_dsoPath[DSOPATH_MAX];
char RTOption::_homeDir[PATH_MAX];
char RTOption::_rcName[PATH_MAX];
char RTOption::_suppressedNamelist[SUPPRESSED_NAMELIST_MAX];

void RTOption::init()
{
	// Set everything to defaults
	_audio = true;
	_play = true;
	_record = false;
	_clobber = false;
	_reportClipping = true;
	_checkPeaks = true;
	_exitOnError = false;	// we override this in main.cpp
	_autoLoad = false;
	_fastUpdate = false;
	_requireSampleRate = true;
#ifdef EMBEDDED
	_print = MMP_RTERRORS; // basic level for max/msp
#else
	_print = MMP_PRINTALL; // default print everthing for regular RTcmix
#endif
    _printListLimit = DEFAULT_PRINT_LIST_LIMIT;
    _parserWarnings = DEFAULT_PARSER_WARNINGS;
	_bufferFrames = DEFAULT_BUFFER_FRAMES;
	_bufferCount = DEFAULT_BUFFER_COUNT;
	_oscInPort = DEFAULT_OSC_INPORT;
	_muteThreshold = DEFAULT_MUTE_THRESHOLD;

	_device[0] = 0;
	_inDevice[0] = 0;
	_outDevice[0][0] = 0;
	_outDevice[1][0] = 0;
	_outDevice[2][0] = 0;
	_midiInDevice[0] = 0;
	_midiOutDevice[0] = 0;
	strcpy(_oscHost, "localhost");
	_dsoPath[0] = 0;
	_homeDir[0] = 0;
	_rcName[0] = 0;
    _suppressedNamelist[0] = 0;

	// initialize home directory and full path of user's configuration file

	char *dir = getenv("HOME");
	if (dir == NULL || strlen(dir) > 256) { // legit HOME not likely to be NULL or longer than this
        dir = (char *) "/tmp";   // fake it to avoid crashes later
    }
	strncpy(_homeDir, dir, PATH_MAX);
	_homeDir[PATH_MAX - 1] = 0;

    size_t rclen = strlen(dir) + 1 + strlen(CONF_FILENAME) + 1;
	char *rc = new char[rclen];
	strncpy(rc, dir, rclen);
	strncat(rc, "/", rclen);
	strncat(rc, CONF_FILENAME, rclen);
	strncpy(_rcName, rc, PATH_MAX);
	_rcName[PATH_MAX - 1] = 0;
	delete [] rc;
}

/* Read configuration file <fileName>, which is probably the full path name
   of the user's .rtcmixrc file, and copy the settings in the file to the
   RTOption object.  Return 0 if successful, or if the file doesn't exist.
   Return -1 if there is a problem reading the file (such as insufficient
   privileges).
*/
int RTOption::readConfigFile(const char *fileName)
{
	const char *key;
	Config conf;

	assert(fileName != NULL && fileName[0] != 0);
	ConfigErrorCode result = conf.parseFile(fileName);
	if (result == kConfigFileMissingErr)
		return -1;			// user doesn't have an rc file; fail silently
	if (result != kConfigNoErr) {
		reportError("%s \"%s\"", conf.getLastErrorText(), fileName);
		return -1;
	}

	// bool options ...........................................................

	bool bval;

	key = kOptionAudio;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		audio(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionPlay;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		play(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionRecord;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		record(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionClobber;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		clobber(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionReportClipping;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		reportClipping(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionCheckPeaks;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		checkPeaks(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionExitOnError;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		exitOnError(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionAutoLoad;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		autoLoad(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionFastUpdate;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		fastUpdate(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionRequireSampleRate;
	result = conf.getValue(key, bval);
	if (result == kConfigNoErr)
		requireSampleRate(bval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

    key = kOptionPrintSuppressUnderbar;
    result = conf.getValue(key, bval);
    if (result == kConfigNoErr)
        printSuppressUnderbar(bval);
    else if (result != kConfigNoValueForKey)
        reportError("%s: %s.", conf.getLastErrorText(), key);

    key = kOptionBailOnUndefinedFunction;
    result = conf.getValue(key, bval);
    if (result == kConfigNoErr)
        bailOnUndefinedFunction(bval);
    else if (result != kConfigNoValueForKey)
        reportError("%s: %s.", conf.getLastErrorText(), key);

    // number options .........................................................

	double dval;

	key = kOptionBufferFrames;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		bufferFrames(dval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionBufferCount;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		bufferCount((int)dval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionPrint;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		print((int)dval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

    key = kOptionPrintListLimit;
    result = conf.getValue(key, dval);
    if (result == kConfigNoErr)
        printListLimit((int)dval);
    else if (result != kConfigNoValueForKey)
        reportError("%s: %s.", conf.getLastErrorText(), key);

    key = kOptionOSCInPort;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		oscInPort((int)dval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionMuteThreshold;
	result = conf.getValue(key, dval);
	if (result == kConfigNoErr)
		muteThreshold(dval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

    key = kOptionParserWarnings;
    result = conf.getValue(key, dval);
    if (result == kConfigNoErr)
        parserWarnings((int)dval);
    else if (result != kConfigNoValueForKey)
        reportError("%s: %s.", conf.getLastErrorText(), key);

	// string options .........................................................

	char *sval;

	key = kOptionDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		device(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionInDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		inDevice(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionOutDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		outDevice(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionOutDevice2;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		outDevice(sval, 1);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionOutDevice3;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		outDevice(sval, 2);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionMidiInDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		midiInDevice(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionMidiOutDevice;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		midiOutDevice(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionOSCHost;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		oscHost(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	key = kOptionDSOPath;
	result = conf.getValue(key, sval);
	if (result == kConfigNoErr)
		dsoPath(sval);
	else if (result != kConfigNoValueForKey)
		reportError("%s: %s.", conf.getLastErrorText(), key);

	return 0;
}

/* This is designed to be called from a utility program that writes a new
   configuration file.
*/
#include <stdio.h>
#include <errno.h>

int RTOption::writeConfigFile(const char *fileName)
{
	if (fileName == NULL || fileName[0] == 0) {
		fprintf(stderr, "Config file name is NULL or empty.\n");
		return -1;
	}

	FILE *stream = fopen(fileName, "r");
	if (stream != NULL || errno != ENOENT) {
		fprintf(stderr, "Config file \"%s\" already exists. Move it out of "
		                "the way.\n", fileName);
		return -1;
	}

	stream = fopen(fileName, "w");
	if (stream == NULL) {
		fprintf(stderr, "Can't open \"%s\" for writing.\n", fileName);
		return -1;
	}

	fprintf(stream, "# Default configuration file\n");

	// write bool options
	fprintf(stream, "\n# Boolean options: key = [true | false]\n");
	fprintf(stream, "%s = %s\n", kOptionAudio, audio() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionPlay, play() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionRecord, record() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionClobber, clobber() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionReportClipping,
										reportClipping() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionCheckPeaks,
										checkPeaks() ? "true" : "false");
	// intentionally leaving out exitOnError
	fprintf(stream, "%s = %s\n", kOptionAutoLoad, 
										autoLoad() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionFastUpdate, 
										fastUpdate() ? "true" : "false");
	fprintf(stream, "%s = %s\n", kOptionRequireSampleRate,
										requireSampleRate() ? "true" : "false");
    fprintf(stream, "%s = %s\n", kOptionPrintSuppressUnderbar,
                                        printSuppressUnderbar() ? "true" : "false");
    fprintf(stream, "%s = %s\n", kOptionBailOnUndefinedFunction,
            bailOnUndefinedFunction() ? "true" : "false");

	// write number options
	fprintf(stream, "\n# Number options: key = value\n");
	fprintf(stream, "%s = %g\n", kOptionBufferFrames, bufferFrames());
	fprintf(stream, "%s = %d\n", kOptionBufferCount, bufferCount());
	fprintf(stream, "%s = %d\n", kOptionOSCInPort, oscInPort());
	fprintf(stream, "%s = %d\n", kOptionPrint, print());
    fprintf(stream, "%s = %d\n", kOptionPrintListLimit, printListLimit());
	fprintf(stream, "%s = %g\n", kOptionMuteThreshold, muteThreshold());

	// write string options
	fprintf(stream, "\n# String options: key = \"quoted string\"\n");
	if (device()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionDevice, device());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionDevice, "mydevice");
	if (inDevice()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionInDevice, inDevice());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionInDevice, "myindevice");
	if (outDevice()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionOutDevice, outDevice());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionOutDevice, "myoutdevice");
	if (midiInDevice()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionMidiInDevice, midiInDevice());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionMidiInDevice,
														"my midi indevice");
	if (midiOutDevice()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionMidiOutDevice, midiOutDevice());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionMidiOutDevice,
														"my midi outdevice");
#ifdef NOTYET  // we haven't implemented OSC transmit, which would need this
	if (oscHost()[0])
		fprintf(stream, "%s = \"%s\"\n", kOptionOSCHost, oscHost());
	else
		fprintf(stream, "# %s = \"%s\"\n", kOptionOSCHost, "localhost");
#endif
#ifdef SHAREDLIBDIR
	fprintf(stream, "\n# %s is a colon-separated list of directories (full "
			"path names) to \n# search for instruments.\n", kOptionDSOPath);
	fprintf(stream, "# %s = \"%s\"\n", kOptionDSOPath, SHAREDLIBDIR);
#endif

	fprintf(stream, "\n");
	fclose(stream);

	return 0;
}


// String option setting methods

char *RTOption::device(const char *devName)
{
	strncpy(_device, devName, DEVICE_MAX);
	_device[DEVICE_MAX - 1] = 0;
	return _device;
}

char *RTOption::inDevice(const char *devName)
{
	strncpy(_inDevice, devName, DEVICE_MAX);
	_inDevice[DEVICE_MAX - 1] = 0;
	return _inDevice;
}

char *RTOption::outDevice(const char *devName, int devIndex)
{
	strncpy(_outDevice[devIndex], devName, DEVICE_MAX);
	_outDevice[devIndex][DEVICE_MAX - 1] = 0;
	return _outDevice[devIndex];
}

char *RTOption::midiInDevice(const char *devName)
{
	strncpy(_midiInDevice, devName, DEVICE_MAX);
	_midiInDevice[DEVICE_MAX - 1] = 0;
	return _midiInDevice;
}

char *RTOption::midiOutDevice(const char *devName)
{
	strncpy(_midiOutDevice, devName, DEVICE_MAX);
	_midiOutDevice[DEVICE_MAX - 1] = 0;
	return _midiOutDevice;
}

char *RTOption::oscHost(const char *oscHost)
{
	strncpy(_oscHost, oscHost, OSCHOST_MAX);
	_oscHost[OSCHOST_MAX - 1] = 0;
	return _oscHost;
}

char *RTOption::dsoPath(const char *pathName)
{
	strncpy(_dsoPath, pathName, DSOPATH_MAX);
	_dsoPath[DSOPATH_MAX - 1] = 0;
	return _dsoPath;
}

char *RTOption::dsoPathPrepend(const char *pathName)
{
    size_t strlength = strlen(pathName) + 1 + strlen(_dsoPath) + 1;
	char *str = new char[strlength];
	strncpy(str, pathName, strlength);
	if (strlen(_dsoPath)) {
		strncat(str, ":", strlength);
		strncat(str, _dsoPath, strlength);
	}
	strncpy(_dsoPath, str, DSOPATH_MAX);
	_dsoPath[DSOPATH_MAX - 1] = 0;
	delete [] str;
	return _dsoPath;
}

char *RTOption::dsoPathAppend(const char *pathName)
{
    size_t strlength = strlen(_dsoPath) + 1 + strlen(pathName) + 1;
	char *str = new char[strlength];
	strncpy(str, _dsoPath, strlength);
	if (strlen(_dsoPath))
		strncat(str, ":", strlength);
	strncat(str, pathName, strlength);
	strncpy(_dsoPath, str, DSOPATH_MAX);
	_dsoPath[DSOPATH_MAX - 1] = 0;
	delete [] str;
	return _dsoPath;
}

char *RTOption::rcName(const char *rcName)
{
	strncpy(_rcName, rcName, PATH_MAX);
	_rcName[PATH_MAX - 1] = 0;
	return _rcName;
}

char *RTOption::suppressedFunNamelist(const char *nameList)
{
    strncpy(_suppressedNamelist, nameList, SUPPRESSED_NAMELIST_MAX-1);
    strncat(_suppressedNamelist, ",", SUPPRESSED_NAMELIST_MAX);
    _suppressedNamelist[SUPPRESSED_NAMELIST_MAX - 1] = 0;
    return _suppressedNamelist;
}


void RTOption::dump()
{
#ifndef EMBEDDED
	cout << kOptionAudio << ": " << _audio << endl;
	cout << kOptionPlay << ": " << _play << endl;
	cout << kOptionRecord << ": " << _record << endl;
	cout << kOptionClobber << ": " << _clobber << endl;
	cout << kOptionPrint << ": " << _print << endl;
	cout << kOptionReportClipping << ": " << _reportClipping << endl;
	cout << kOptionCheckPeaks << ": " << _checkPeaks << endl;
	cout << kOptionExitOnError << ": " << _exitOnError << endl;
	cout << kOptionAutoLoad << ": " << _autoLoad << endl;
	cout << kOptionFastUpdate << ": " << _fastUpdate << endl;
	cout << kOptionRequireSampleRate << ": " << _requireSampleRate << endl;
    cout << kOptionPrintSuppressUnderbar << ": " << _printSuppressUnderbar << endl;
    cout << kOptionBailOnUndefinedFunction << ": " << _bailOnUndefinedFunction << endl;
	cout << kOptionBufferFrames << ": " << _bufferFrames << endl;
	cout << kOptionBufferCount << ": " << _bufferCount << endl;
    cout << kOptionPrintListLimit << ": " << _printListLimit << endl;
	cout << kOptionMuteThreshold << ": " << _muteThreshold << endl;
	cout << kOptionOSCInPort << ": " << _oscInPort << endl;
	cout << kOptionDevice << ": " << _device << endl;
	cout << kOptionInDevice << ": " << _inDevice << endl;
	cout << kOptionOutDevice << ": " << _outDevice[0] << endl;
	cout << kOptionOutDevice2 << ": " << _outDevice[1] << endl;
	cout << kOptionOutDevice3 << ": " << _outDevice[2] << endl;
	cout << kOptionMidiInDevice << ": " << _midiInDevice << endl;
	cout << kOptionMidiOutDevice << ": " << _midiOutDevice << endl;
	cout << kOptionOSCHost << ": " << _oscHost << endl;
	cout << kOptionDSOPath << ": " << _dsoPath << endl;
	cout << kOptionRCName << ": " << _rcName << endl;
	cout << kOptionHomeDir << ": " << _homeDir << endl;
#endif // EMBEDDED
}

void RTOption::reportError(const char *format, const char *msg1, const char *msg2)
{
	char buf[1024];
	snprintf(buf, 1024, format, msg1, msg2);
	printf("Config file error:  %s\n", buf);
}


// ----------------------------------------------------------------------------
// These functions are for C code that needs to query options.

int get_print_option()
{
	return RTOption::print();
}

char *get_dsopath_option()
{
	return RTOption::dsoPath();
}

int get_bool_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionReportClipping))
		return (int) RTOption::reportClipping();
	else if (!strcmp(option_name, kOptionCheckPeaks))
		return (int) RTOption::checkPeaks();
	else if (!strcmp(option_name, kOptionClobber))
		return (int) RTOption::clobber();
	else if (!strcmp(option_name, kOptionAudio))
		return (int) RTOption::audio();
	else if (!strcmp(option_name, kOptionPlay))
		return (int) RTOption::play();
	else if (!strcmp(option_name, kOptionRecord))
		return (int) RTOption::record();
	else if (!strcmp(option_name, kOptionExitOnError))
		return (int) RTOption::exitOnError();
    else if (!strcmp(option_name, kOptionBailOnError))
        return (int) RTOption::bailOnError();
    else if (!strcmp(option_name, kOptionBailOnParserWarning))
        return (int) RTOption::bailOnParserWarning();
	else if (!strcmp(option_name, kOptionAutoLoad))
		return (int) RTOption::autoLoad();
	else if (!strcmp(option_name, kOptionFastUpdate))
		return (int) RTOption::fastUpdate();
	else if (!strcmp(option_name, kOptionRequireSampleRate))
		return (int) RTOption::requireSampleRate();
    else if (!strcmp(option_name, kOptionPrintSuppressUnderbar))
        return (int) RTOption::printSuppressUnderbar();
    else if (!strcmp(option_name, kOptionBailOnUndefinedFunction))
        return (int)RTOption::bailOnUndefinedFunction();
    else if (!strcmp(option_name, kOptionSendMIDIRecordAutoStart))
        return (int)RTOption::sendMIDIRecordAutoStart();

	assert(0 && "unsupported option name");		// program error
	return 0;
}

void set_bool_option(const char *option_name, int value)
{
	if (!strcmp(option_name, kOptionReportClipping))
		RTOption::reportClipping((bool) value);
	else if (!strcmp(option_name, kOptionCheckPeaks))
		RTOption::checkPeaks((bool) value);
	else if (!strcmp(option_name, kOptionClobber))
		RTOption::clobber((bool) value);
	else if (!strcmp(option_name, kOptionAudio))
		RTOption::audio((bool) value);
	else if (!strcmp(option_name, kOptionPlay))
		RTOption::play((bool) value);
	else if (!strcmp(option_name, kOptionRecord))
		RTOption::record((bool) value);
	else if (!strcmp(option_name, kOptionExitOnError))
		RTOption::exitOnError((bool) value);
    else if (!strcmp(option_name, kOptionBailOnError))
        RTOption::bailOnError((bool) value);
    else if (!strcmp(option_name, kOptionBailOnParserWarning))
        RTOption::bailOnParserWarning((bool) value);
	else if (!strcmp(option_name, kOptionAutoLoad))
		RTOption::autoLoad((bool) value);
	else if (!strcmp(option_name, kOptionFastUpdate))
		RTOption::fastUpdate((bool) value);
	else if (!strcmp(option_name, kOptionRequireSampleRate))
		RTOption::requireSampleRate((bool) value);
    else if (!strcmp(option_name, kOptionPrintSuppressUnderbar))
        RTOption::printSuppressUnderbar((bool) value);
    else if (!strcmp(option_name, kOptionSendMIDIRecordAutoStart))
        RTOption::sendMIDIRecordAutoStart((bool)value);
	else
		assert(0 && "unsupported option name");
}

double get_double_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionBufferFrames))
		return RTOption::bufferFrames();
	else if (!strcmp(option_name, kOptionBufferCount))
		return RTOption::bufferCount();
	else if (!strcmp(option_name, kOptionPrint))
		return RTOption::print();
    else if (!strcmp(option_name, kOptionParserWarnings))
        return RTOption::parserWarnings();
	else if (!strcmp(option_name, kOptionMuteThreshold))
		return RTOption::muteThreshold();

	assert(0 && "unsupported option name");
	return 0;
}

void set_double_option(const char *option_name, double value)
{
	if (!strcmp(option_name, kOptionBufferFrames))
		RTOption::bufferFrames(value);
	else if (!strcmp(option_name, kOptionBufferCount))
		RTOption::bufferCount((int)value);
	else if (!strcmp(option_name, kOptionPrint))
		RTOption::print((int)value);
    else if (!strcmp(option_name, kOptionParserWarnings))
        RTOption::parserWarnings((int)value);
	else if (!strcmp(option_name, kOptionMuteThreshold))
		RTOption::muteThreshold(value);
	else
		assert(0 && "unsupported option name");
}

char *get_string_option(const char *option_name)
{
	if (!strcmp(option_name, kOptionDevice))
		return RTOption::device();
	else if (!strcmp(option_name, kOptionInDevice))
		return RTOption::inDevice();
	else if (!strcmp(option_name, kOptionOutDevice))
		return RTOption::outDevice();
	else if (!strcmp(option_name, kOptionOutDevice2))
		return RTOption::outDevice(1);
	else if (!strcmp(option_name, kOptionOutDevice3))
		return RTOption::outDevice(2);
	else if (!strcmp(option_name, kOptionDSOPath))
		return RTOption::dsoPath();

	assert(0 && "unsupported option name");
	return 0;
}

void set_string_option(const char *option_name, const char *value)
{
	if (!strcmp(option_name, kOptionDevice))
		RTOption::device(value);
	else if (!strcmp(option_name, kOptionInDevice))
		RTOption::inDevice(value);
	else if (!strcmp(option_name, kOptionOutDevice))
		RTOption::outDevice(value);
	else if (!strcmp(option_name, kOptionOutDevice2))
		RTOption::outDevice(value, 1);
	else if (!strcmp(option_name, kOptionOutDevice3))
		RTOption::outDevice(value, 2);
	else if (!strcmp(option_name, kOptionDSOPath))
		RTOption::dsoPath(value);
	else
		assert(0 && "unsupported option name");
}

// This is so we can call dump from within GDB.
void option_dump()
{
	RTOption::dump();
}

