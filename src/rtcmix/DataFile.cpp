/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <DataFile.h>
#include <errno.h>
#include <string.h>


DataFile::DataFile(const char *fileName, const int controlRate)
	: _stream(NULL), _swap(false),
	  _format(kDataFormatFloat), _datumsize(sizeof(float)),
	  _controlrate(controlRate), _filerate(0), _increment(1.0), _counter(1.0),
	  _lastval(0.0)
{
	_filename = strdup(fileName);
}

DataFile::~DataFile()
{
	closeFile();
	delete _filename;
}

int DataFile::openFileWrite(const bool clobber)
{
	_stream = fopen(_filename, "w");
	if (_stream == NULL) {
		fprintf(stderr, "Can't open data file \"%s\" for writing: %s\n",
						_filename, strerror(errno));
		return -1;
	}
	return 0;
}

int DataFile::openFileRead()
{
	_stream = fopen(_filename, "r");
	if (_stream == NULL) {
		fprintf(stderr, "Can't open data file \"%s\" for reading: %s\n",
						_filename, strerror(errno));
		return -1;
	}
	return 0;
}

int DataFile::closeFile()
{
	int status = 0;
	if (_stream) {
		if (fclose(_stream) != 0) {
			fprintf(stderr, "Error closing data file \"%s\": %s\n",
					_filename, strerror(errno));
			status = -1;
		}
		_stream = NULL;
	}
	return status;
}

// Return number of bytes to hold one element of given format.
size_t format_datumsize(const int format)
{
	switch (format) {
		case kDataFormatDouble: return sizeof(double); break;
		case kDataFormatFloat:  return sizeof(float); break;
		case kDataFormatInt64:  return sizeof(int64_t); break;
		case kDataFormatInt32:  return sizeof(int32_t); break;
		case kDataFormatInt16:  return sizeof(int16_t); break;
		case kDataFormatByte:   return sizeof(int8_t); break;
	}
	return 0;
}

// Return a string describing the given format.
const char *format_string(const int format)
{
	switch (format) {
		case kDataFormatDouble: return "doubles"; break;
		case kDataFormatFloat:  return "floats"; break;
		case kDataFormatInt64:  return "64-bit signed integers"; break;
		case kDataFormatInt32:  return "32-bit signed integers"; break;
		case kDataFormatInt16:  return "16-bit signed integers"; break;
		case kDataFormatByte:   return "8-bit signed bytes"; break;
	}
	return "";
}

int DataFile::writeHeader(const int fileRate, const int format, const bool swap)
{
	_format = format;
	_datumsize = format_datumsize(_format);
	_filerate = fileRate;
	_increment = double(_controlrate) / double(_filerate);
	_swap = swap;

	int32_t magic = _swap ? kMagicSwapped : kMagic;
	int32_t fmt = _format;
	int32_t rate = _filerate;

	size_t nitems = fwrite(&magic, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;

	nitems = fwrite(&fmt, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;

	nitems = fwrite(&rate, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;

	return 0;
err_return:
	fprintf(stderr, "Error writing header for data file \"%s\"\n", _filename);
	return -1;
}

// Attempt to interpret first few ints as a header.  If successful, return 0;
// if not, return -1, rewind the file, and use the default values.
int DataFile::readHeader(
		const int   defaultFileRate,
		const int   defaultFormat,
		const bool  defaultSwap)
{
	int32_t magic;
	size_t nitems = fread(&magic, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;
	if (magic == kMagic)
		_swap = false;
	else if (magic == kMagicSwapped)
		_swap = true;
	else
		goto err_return;

	int32_t format;
	nitems = fread(&format, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;

	int32_t filerate;
	nitems = fread(&filerate, sizeof(int32_t), 1, _stream);
	if (nitems != 1)
		goto err_return;

	_format = format;
	_datumsize = format_datumsize(_format);
	_filerate = filerate;
	_increment = double(_controlrate) / double(_filerate);
	return 0;

err_return:
	_swap = defaultSwap;
	_format = defaultFormat;
	_datumsize = format_datumsize(_format);
	_filerate = (defaultFileRate == -1) ? _controlrate : defaultFileRate;
	_increment = double(_controlrate) / double(_filerate);
	printf("No header for data file \"%s\";\n"
				"assuming %s at %d per second, %s.\n",
				_filename, format_string(_format), _filerate,
				_swap ? "with byte-swapping" : "no byte-swapping");
	rewind(_stream);
	return -1;
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
			fprintf(stderr, "Error writing data file \"%s\"\n", _filename);
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

