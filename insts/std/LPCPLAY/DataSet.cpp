// DataSet.C

#include <ugens.h>
#include "DataSet.h"
#include <stdio.h>
#include <unistd.h>		// for lseek
#include <sys/stat.h>	// for stat for getFrame()
#include <fcntl.h>

#include "lp.h"

DataSet::DataSet()
	: _refCount(0), _nPoles(0), _fdesc(-1), _lpHeaderSize(0), _array(NULL),
	_oldframe(0), _endframe(0)
{
	_fprec = 22;
}

DataSet::~DataSet()
{
	if (_fdesc > 0)
		::close(_fdesc);
	delete [] _array;
}

void
DataSet::ref()
{
	++_refCount;
}

void
DataSet::unref()
{
	if (this == NULL)
		return;		// special trick to allow call on NULL object
	if (--_refCount <= 0)
		delete this;
}

int
DataSet::open(const char *fileName, int npoleGuess, float sampRate)
{
	_nPoles = npoleGuess;	// in case we are not using headers
    if ((_fdesc = ::open(fileName, O_RDONLY)) < 0) {
		::die("dataset", "Can't open %s", fileName);
    }
	::advise("dataset", "Opened lpc dataset %s.", fileName);
#ifdef USE_HEADERS
	if ((_lpHeaderSize = ::checkForHeader(_fdesc, &_nPoles, sampRate)) < 0)
	    ::die("dataset", "Failed to check header");
#else
	if (!_nPoles) {
		return -1;
	}
#endif /* USE_HEADERS */

	allocArray(_nPoles);

	_npolem1=_nPoles-1;
	_framsize=_nPoles+4;
	_recsize=_fprec*_framsize;
	_bprec=_recsize*FLOAT;
	_bpframe=_framsize*FLOAT;

	struct stat st;
	/* return number of frames in datafile */
	if (::stat(fileName, &st) >= 0) {
		int frms = (st.st_size-_lpHeaderSize) / _bpframe;
		return frms;
	}
	else {
		::die("dataset", "Unable to stat dataset file.");
		return 0;	/* not reached */
	}
}

int
DataSet::getFrame(float frameno, float *pCoeffs)
{
	int i,j;
	int frame = (int)frameno;
	float fraction = frameno - (float)frame;
	if (!((frame >= _oldframe) && (frame < _endframe))) {
		int bytesRead, framesRead = _fprec;
    	if (::lseek(_fdesc, _lpHeaderSize+(frame*_bpframe), 0) == -1)
		{
            	fprintf(stderr,"bad lseek on analysis file \n");
            	return(-1);
    	}
#ifdef debug
		printf("Reading frames %d - %d from file\n", frame, frame + 22);
#endif
		/* Quit if we read less than one complete frame */
    	if ((bytesRead = ::read(_fdesc, _array, _bprec)) < _bpframe)
		{
            	fprintf(stderr,"reached eof on analysis file \n");
            	return(-1);
    	}
		framesRead = bytesRead / _bpframe;
    	_oldframe = frame;
    	_endframe = _oldframe + framesRead - 1;
	}
	for(i=(frame-_oldframe)*_framsize,j=0; j<_framsize; i++,j++)  
        	pCoeffs[j] = _array[i] + fraction * (_array[i+_framsize] - _array[i]);
	return(0);
}

void
DataSet::allocArray(int nPoles)
{
	if (_array)
	    delete [] _array;
	_array = new float[_fprec * (nPoles + 4)];
}
