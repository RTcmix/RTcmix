// DataSet.C

#include <ugens.h>
#include "DataSet.h"
#include <stdio.h>
#include <unistd.h>		// for lseek
#include <sys/stat.h>	// for stat for getFrame()
#include <fcntl.h>
#include <byte_routines.h>
#include "lpcdefs.h"

DataSet::DataSet()
	: _nPoles(0), _frameCount(0), _fdesc(-1), _lpHeaderSize(0), 
	  _array(NULL), _oldframe(0), _endframe(0), _swapped(false)
{
	_fprec = 22;
}

DataSet::~DataSet()
{
	if (_fdesc > 0)
		::close(_fdesc);
	delete [] _array;
}

off_t
DataSet::open(const char *fileName, int npoleGuess, float sampRate)
{
    if ((_fdesc = ::open(fileName, O_RDONLY)) < 0) {
		return die("dataset", "Can't open %s", fileName);
    }
	_nPoles = npoleGuess;	// in case we are not using headers
	::rtcmix_advise("dataset", "Opened lpc dataset %s.", fileName);
	Bool isSwapped = NO;
	if ((_lpHeaderSize = ::checkForHeader(_fdesc, &_nPoles, sampRate, &isSwapped)) < 0) {
	    return die("dataset", "Failed to check header");
	}
	_swapped = (isSwapped != 0);

	allocArray(_nPoles);

	_npolem1=_nPoles-1;
	_framsize=_nPoles+4;
	_recsize=_fprec*_framsize;
	_bprec=_recsize*FLOAT;
	_bpframe=_framsize*FLOAT;

	struct stat st;
	/* store and return number of frames in datafile */
	if (::stat(fileName, &st) >= 0) {
		_frameCount = (st.st_size-_lpHeaderSize) / _bpframe;
		return _frameCount;
	}
	else {
		return die("dataset", "Unable to stat dataset file.");
	}
}

int
DataSet::getFrame(double frameno, float *pCoeffs)
{
	int i,j;
	int frame = (int)frameno;
	double fraction = frameno - (double)frame;
	if (!((frame >= _oldframe) && (frame < _endframe))) {
		int bytesRead, framesRead = _fprec;
    	if (::lseek(_fdesc, _lpHeaderSize+(frame*_bpframe), 0) == -1)
		{
            	rtcmix_warn("LPC", "bad lseek on analysis file");
            	return(-1);
    	}
#ifdef debug
		printf("Reading frames %d - %d from file\n", frame, frame + 22);
#endif
		/* Quit if we read less than one complete frame */
    	if ((bytesRead = ::read(_fdesc, _array, _bprec)) < _bpframe)
		{
            	rtcmix_warn("LPC","reached eof on analysis file");
            	return(-1);
    	}
		framesRead = bytesRead / _bpframe;
    	_oldframe = frame;
    	_endframe = _oldframe + framesRead - 1;
	}
	for(i=(frame-_oldframe)*_framsize,j=0; j<_framsize; i++,j++) {
		float first = _array[i];
		float second = _array[i+_framsize];
		if (_swapped) {
			byte_reverse4(&first);
			byte_reverse4(&second);
		}
		pCoeffs[j] = first + fraction * (second - first);
	}
	return(0);
}

void
DataSet::allocArray(int nPoles)
{
	if (_array)
	    delete [] _array;
	_array = new float[_fprec * (nPoles + 4)];
}
