//
//  RawDataFile.cpp
//  RTcmix
//
//  Created by Douglas Scott on 4/26/13.
//
//

#include "RawDataFile.h"

#include "DataFile.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ugens.h>	// for message.c functions

RawDataFile::RawDataFile(const char *fileName)
	: _stream(NULL), _swap(false)
{
	_filename = strdup(fileName);
}

RawDataFile::~RawDataFile()
{
	closeFile();
	delete _filename;
}

int RawDataFile::openFileWrite(const bool clobber)
{
	_stream = fopen(_filename, "w");
	if (_stream == NULL) {
		rterror(NULL, "Can't open data file \"%s\" for writing: %s\n",
				_filename, strerror(errno));
		return -1;
	}
	return 0;
}

int RawDataFile::openFileRead()
{
	_stream = fopen(_filename, "r");
	if (_stream == NULL) {
		rterror(NULL, "Can't open data file \"%s\" for reading: %s\n",
				_filename, strerror(errno));
		return -1;
	}
	return 0;
}

int RawDataFile::closeFile()
{
	int status = 0;
	if (_stream) {
		if (fclose(_stream) != 0) {
			rterror(NULL, "Error closing data file \"%s\": %s\n",
					_filename, strerror(errno));
			status = -1;
		}
		_stream = NULL;
	}
	return status;
}
