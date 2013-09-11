/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// Class for reading and writing data files at the control rate, taking into
// account the possibility that the control rate stored in the (optional)
// file header may not be the same.  This is designed primarily for recording
// PField automation data.  -JGG, 3/15/05

#ifndef _DATAFILE_H_
#define _DATAFILE_H_

#include "RawDataFile.h"
#include <stdint.h>
#include <stdio.h>

const int32_t kMagic        = 0x636D6978;  // "cmix"
const int32_t kMagicSwapped = 0x78696D63;  // "ximc"

enum {
	kDataFormatDouble,
	kDataFormatFloat,
	kDataFormatInt64,
	kDataFormatInt32,
	kDataFormatInt16,
	kDataFormatByte
};


// Data file header consists of...
//
// int32_t magic         kMagic or kMagicSwapped
// int32_t format        see enum above
// int32_t controlrate   control rate in effect when file was saved

const int kHeaderSize = sizeof(int32_t) * 3;		// header length in bytes


class DataFile : public RawDataFile {
public:
	DataFile(const char *fileName, const int controlRate = 1000,
					const double timeFactor = 1.0);
	virtual ~DataFile();

	int writeHeader(const int fileRate, const int format, const bool swap);
	long readHeader(const int  defaultFileRate = -1,
						const int  defaultFormat = kDataFormatFloat,
						const bool defaultSwap = false);

	// Call setSkipTime after readHeader to start reading somewhere other than at
	// the beginning of the file.  <skipTime> is the duration to skip, computed
	// with reference to the file rate, and prior to any time-scaling of the
	// data.  If <absolute> is true, skip from the end of the header; otherwise,
	// skip relative to the current position.  setSkipTime can be called any
	// number of times, as long as readHeader has already set a valid file rate.
	// Returns -1 if seek error (and it reports error); 0 otherwise.

	int setSkipTime(const double skipTime, const bool absolute = false);

	int writeOne(const double val);
	double readOne();
	int readFile(double *block, const long maxItems);

	static int formatStringToCode(const char *str);

private:
	int _headerbytes;
	int _format;
	int _datumsize;
	long _fileitems;
	int _controlrate;
	int _filerate;
	double _timefactor;
	double _increment;
	double _counter;
	double _lastval;		// used for reading only
};

#endif // _DATAFILE_H_
