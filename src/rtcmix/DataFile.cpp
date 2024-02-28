/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "DataFile.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ugens.h>	// for message.c functions


DataFile::DataFile(const char *fileName, const int controlRate,
				   const double timeFactor)
	: RawDataFile(fileName),
	  _headerbytes(0),
	  _format(kDataFormatFloat), _datumsize(sizeof(float)), _fileitems(0),
	  _controlrate(controlRate), _filerate(0), _timefactor(timeFactor),
	  _increment(1.0), _counter(1.0), _lastval(0.0)
{
}

DataFile::~DataFile()
{
}

int DataFile::formatStringToCode(const char *str)
{
	if (str == NULL)
		return -1;
   if (strcmp(str, "double") == 0)
      return kDataFormatDouble;
   else if (strcmp(str, "float") == 0)
      return kDataFormatFloat;
   else if (strcmp(str, "int") == 0)
       return kDataFormatInt32;
   else if (strcmp(str, "int64") == 0)
       return kDataFormatInt64;
   else if (strcmp(str, "int32") == 0)
       return kDataFormatInt32;
   else if (strcmp(str, "int16") == 0)
       return kDataFormatInt16;
   else if (strcmp(str, "byte") == 0)
       return kDataFormatByte;
   return -1;
}

// Return number of bytes to hold one element of given format, or -1 if
// format is invalid.
static int format_datumsize(const int format)
{
	switch (format) {
		case kDataFormatDouble: return sizeof(double);
		case kDataFormatFloat:  return sizeof(float);
		case kDataFormatInt64:  return sizeof(int64_t);
		case kDataFormatInt32:  return sizeof(int32_t);
		case kDataFormatInt16:  return sizeof(int16_t);
		case kDataFormatByte:   return sizeof(int8_t);
		default:                return -1;
	}
}

// Return a string describing the given format.
static const char *format_string(const int format)
{
	switch (format) {
		case kDataFormatDouble: return "doubles";
		case kDataFormatFloat:  return "floats";
		case kDataFormatInt64:  return "64-bit signed integers";
		case kDataFormatInt32:  return "32-bit signed integers";
		case kDataFormatInt16:  return "16-bit signed integers";
		case kDataFormatByte:   return "8-bit signed bytes";
	}
	return "";
}

int DataFile::writeHeader(const int fileRate, const int format, const bool swap)
{
	_format = format;
	_datumsize = format_datumsize(_format);
	assert(_datumsize != -1);
	_filerate = fileRate;
	_increment = (double(_controlrate) / double(_filerate)) / _timefactor;
	_swap = swap;

	int32_t magic = _swap ? kMagicSwapped : kMagic;
	int32_t fmt = _format;
	int32_t rate = _filerate;

	size_t nitems = fwrite(&magic, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto writeerr;

	nitems = fwrite(&fmt, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto writeerr;

	nitems = fwrite(&rate, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto writeerr;

	_headerbytes = kHeaderSize;

	return 0;
writeerr:
	rterror(NULL, "Error writing header for data file \"%s\"\n", _filename);
	return -1;
}

// Return the number of items in this file.
static inline long fileItems(const long fileBytes, const int datumSize,
	const int headerBytes)
{
	return (fileBytes - headerBytes) / datumSize;
}

// Attempt to interpret first few ints as a header.  If successful, leave
// the file pointer set to the first non-header item.  If not successful,
// rewind the file, and use the default values for the header information.
// In either case, return the number of items in the file.  In case of read
// error, return -1.
long DataFile::readHeader(
		const int   defaultFileRate,
		const int   defaultFormat,
		const bool  defaultSwap)
{
	if (fseek(_stream, 0, SEEK_END) != 0) {
		rterror(NULL, "Seek error for data file \"%s\": %s\n",
					_filename, strerror(errno));
		return -1;
	}
	long filebytes = ftell(_stream);
	rewind(_stream);

	int32_t magic;
	size_t nitems = fread(&magic, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto readerr;
	if (magic == kMagic)
		_swap = false;
	else if (magic == kMagicSwapped)
		_swap = true;
	else {
		_swap = defaultSwap;
		_format = defaultFormat;
		_datumsize = format_datumsize(_format);
		_filerate = (defaultFileRate == -1) ? _controlrate : defaultFileRate;
		_increment = (double(_controlrate) / double(_filerate)) * _timefactor;
		rtcmix_advise(NULL, "No header for data file \"%s\";\n"
					"assuming %s at %d per second, %s.\n",
					_filename, format_string(_format), _filerate,
					_swap ? "with byte-swapping" : "no byte-swapping");
		rewind(_stream);
		_fileitems = fileItems(filebytes, _datumsize, _headerbytes);
		return _fileitems;
	}
	_headerbytes = kHeaderSize;

	int32_t format;
	nitems = fread(&format, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto readerr;
	if (_swap)
		format = _swapit(format);

	int32_t filerate;
	nitems = fread(&filerate, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto readerr;
	if (_swap)
		filerate = _swapit(filerate);

	_format = format;
	_datumsize = format_datumsize(_format);
	if (_datumsize == -1)
		goto invaldata;
	_filerate = filerate;
	if (_filerate < 1)
		goto invaldata;
	_increment = (double(_controlrate) / double(_filerate)) * _timefactor;
	_fileitems = fileItems(filebytes, _datumsize, _headerbytes);
	return _fileitems;

readerr:
	if (ferror(_stream))
		rterror(NULL, "Read error for data file \"%s\"\n", _filename);
	else
		rtcmix_warn(NULL, "There's hardly anything in data file \"%s\"!\n",
				_filename);
	return -1;

invaldata:
	rterror(NULL, "Invalid header in data file \"%s\"\n", _filename);
	return -1;
}

int DataFile::setSkipTime(const double skipTime, const bool absolute)
{
	if (skipTime == 0.0)
		return 0;
	const long skipframes = int((_filerate * skipTime) + 0.5);
	long skipbytes = skipframes * _datumsize;
	int whence = absolute ? SEEK_SET : SEEK_CUR;
	if (whence == SEEK_SET)
		skipbytes += _headerbytes;
//XXX If this would seek past end, print warning?
	if (fseek(_stream, skipbytes, whence) != 0) {
		rterror(NULL, "Invalid seek into data file \"%s\": %s\n",
												_filename, strerror(errno));
		return -1;
	}
	return 0;
}

// XXX need to parameterize writeOne and readOne by datafile format and swap,
// but these two are stored as class members.

int DataFile::writeOne(const double val)
{
	int status = 0;

	_counter -= 1.0;				// counting at client control rate
	while (_counter <= 0.0) {
		_counter += _increment;

		switch (_format) {
			case kDataFormatDouble:
				{
					double raw = val;
					if (_swap)
						raw = _swapit(raw);
					status = _write(raw);
				}
				break;
			case kDataFormatFloat:
				{
					float raw = (float) val;
					if (_swap)
						raw = _swapit(raw);
					status = _write(raw);
				}
				break;
			case kDataFormatInt64:
				{
					int64_t raw = (int64_t) val;
					if (_swap)
						raw = _swapit(raw);
					status = _write(raw);
				}
				break;
			case kDataFormatInt32:
				{
					int32_t raw = (int32_t) val;
					if (_swap)
						raw = _swapit(raw);
					status = _write(raw);
				}
				break;
			case kDataFormatInt16:
				{
					int16_t raw = (int16_t) val;
					if (_swap)
						raw = _swapit(raw);
					status = _write(raw);
				}
				break;
			case kDataFormatByte:
				status = _write((int8_t) val);
				break;
			default:
				break;
		}
		if (status != 0)
			rterror(NULL, "Error writing data file \"%s\"\n", _filename);
	}
	return status;
}

double DataFile::readOne()
{
	_counter -= 1.0;				// counting at client control rate
	while (_counter <= 0.0) {
		_counter += _increment;

		int status;
		double val;

		switch (_format) {
			case kDataFormatDouble:
				{
					double raw;
					status = _read(&raw);
					if (_swap)
						raw = _swapit(raw);
					val = raw;
				}
				break;
			case kDataFormatFloat:
				{
					float raw;
					status = _read(&raw);
					if (_swap)
						raw = _swapit(raw);
					val = (double) raw;
				}
				break;
			case kDataFormatInt64:
				{
					int64_t raw;
					status = _read(&raw);
					if (_swap)
						raw = _swapit(raw);
					val = (double) raw;
				}
				break;
			case kDataFormatInt32:
				{
					int32_t raw;
					status = _read(&raw);
					if (_swap)
						raw = _swapit(raw);
					val = (double) raw;
				}
				break;
			case kDataFormatInt16:
				{
					int16_t raw;
					status = _read(&raw);
					if (_swap)
						raw = _swapit(raw);
					val = (double) raw;
				}
				break;
			case kDataFormatByte:
				{
					int8_t raw;
					status = _read(&raw);
					val = (double) raw;
				}
				break;
			default:
				val = 0.0;
				status = -1;
				break;
		}
		// Instead of reporting read error or EOF, we just return _lastval.
		if (status == 0)
			_lastval = val;
	}

	return _lastval;
}

int DataFile::readFile(double *block, const long maxItems)
{
	_increment = 1.0;
	for (int i = 0; i < maxItems; i++)
		block[i] = readOne();
	return 0;
}

