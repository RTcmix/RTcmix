// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.
//
// Wrapper class for IRCAM SDIF Library.

#ifndef _SDIFFILE_H_
#define _SDIFFILE_H_

#include <math.h>		// for fabs()
#include <float.h>	// for DBL_EPSILON
#include "Frame.h"

class SDIFfile {
public:
	SDIFfile(void);
	~SDIFfile(void);

	// Parse the given SDIF file into memory, returning non-zero if there was
	// an error. Can use SDIF selection syntax as part of file name. For a
	// description of this syntax, see
	// http://sdif.sourceforge.net/extern/utilities-main.html.
	int readFile(const char *fileName);

	// Say whether a file has been read and is ready to provide frames.
	bool hasFile(void) const { return _hasFile; }

	// Return number of frames held by SDIFfile object.
	long numFrames(void) const { return _numFrames; }

	// Return maximum partial index.
	int maxPartialID(void) const { return _maxPartialID; }

	// Return maximum number of simultaneous partials.
	int maxSimultaneousPartials(void) const { return _maxSimultaneousPartials; }

	// Return the frame that is closest to <time>, according to the selection
	// criterion. Return NULL if no frame fits the criterion. Frame is allocated
	// inside SDIFfile class, so caller must copy the data out of it before
	// calling getFrame again.
	typedef enum {
		NearestEarlier = 0,
		NearestLater,
		Interpolated
	} SelectionCriterion;
	Frame *getFrame(const double time,
							SelectionCriterion criterion=NearestEarlier);

	// Return an empty frame, with time==0. When passed to 
	// PartialFramePlayer::setFrame() with forceUpdate==true, this can be used
	// to force all partials into the dying state.
	Frame *emptyFrame(void);

	// Analyze the SDIF data across partial tracks. Currently, this involves
	// only storing the average frequency of each partial track.
	// NOTE: this must be done after calling readFile().
	void analyze(void);

	// Exclude partials not lying entirely in the range [minFreq, maxFreq],
	// or whose max amp is below ampThreshold. Takes effect on next call to
	// getFrame.
	void filterPartials(float minFreq, float maxFreq, float ampThreshold);

	// Return a pointer to an array of average frequencies of all the partial
	// tracks in the SDIF file. The array has maxPartialID+1 elements.
	// The memory is allocated and held by the SDIFfile object. If the returned
	// pointer is NULL, it probably means that analyze() has not been called yet.
	float *getAverageFreqs(void) const { return _avgFreqs; }

	// Sort the partials in all frames of two SDIF files.
	// Assumes readFile() has been called on each file.
	static void prepareForMorphing(SDIFfile *sdif1, SDIFfile *sdif2);
	
	// Sort the partial nodes in all frames in ascending order by partial
	// frequency. 
	void sortFramePartialsByFrequency(void)
	{
		for (long i = 0; i < _numFrames; i++)
			_frames[i]->sortPartials();
	}

	// Print all partials of frames in this SDIF file. Frames printed are in
	// the half-open range [firstIndex, lastIndex). If lastIndex is zero, it
	// will be set to one past the last frame index (i.e., maxPartialID()).
	void dumpFrames(long firstIndex=0, long lastIndex=0);

private:
	int _frameAlloc(void);
	int _checkFrameAlloc(void);
	void _copyPartials(const long targetIndex);
	void _copyInterpolatePartials(const double requestedTime,
				const long targetIndex1, const long targetIndex2);

	// For the reasons why this approach is not really adequate, see
	// http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition
	bool _equalDouble(const double a, const double b) const
	{
		return (fabs(a - b) <= DBL_EPSILON);
	}

	// Linearly interpolate between val1 and val2, using <frac> factor, which
	// must be in range [0, 1].
	double _interp(const double frac, const double val1, const double val2) const
	{
		return val1 + ((val2 - val1) * frac);
	}

	bool		_hasFile, _filterChanged, *_filterPartials;
	int		_maxPartialID, _maxSimultaneousPartials;
	long		_numFrames, _framesLen, _lastFrameIndex;
	float		_minFreq, _maxFreq, _ampThreshold;
	float		*_minFreqs;	// min freq for each partial track
	float		*_maxFreqs;	// max freq for each partial track
	float		*_avgFreqs;	// average freq for each partial track
	float		*_maxAmps;	// max amp for each partial track
	Frame		**_frames;	// array of Frame objects
	Frame		*_curFrame;
};

#endif // _SDIFFILE_H_
