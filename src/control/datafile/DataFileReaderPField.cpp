/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <DataFileReaderPField.h>
#include <Ougens.h>	// for Oonepole


DataFileReaderPField::DataFileReaderPField(const char *fileName,
		const double lag, const int controlRate, const int defaultFileRate,
		const int defaultFormat, const bool defaultSwap)
	: RTNumberPField(0)
{
	_datafile = new DataFile(fileName, controlRate);
	if (_datafile) {
		if (_datafile->openFileRead() == 0)
			_datafile->readHeader(defaultFileRate, defaultFormat, defaultSwap);
		else {
			delete _datafile;
			_datafile = NULL;
		}
	}
	_filter = new Oonepole(controlRate);
	_filter->setlag(lag);
}

DataFileReaderPField::~DataFileReaderPField()
{
	delete _datafile;
	delete _filter;
}

double DataFileReaderPField::doubleValue(double) const
{
	return _datafile ? _filter->next(_datafile->readOne()) : 0.0;
}

