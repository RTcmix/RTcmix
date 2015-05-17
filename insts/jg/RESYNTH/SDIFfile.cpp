// Copyright (C) 2012 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "SDIFfile.h"
extern "C" {
#include <sdif.h>
}
#include <stdlib.h>
#include <stdio.h>
#include <float.h>	// for FLT_MAX

//#define DEBUG1	// list filtered partials

const int kDefaultFrames = 20;	// number of frames to allocate at once

SDIFfile::SDIFfile(void)
	: _hasFile(false), _filterChanged(true), _filterPartials(NULL),
	  _maxPartialID(0), _maxSimultaneousPartials(0), _numFrames(0),
	  _framesLen(0), _lastFrameIndex(-1),
	  _minFreq(0.0), _maxFreq(FLT_MAX), _ampThreshold(0.0),
	  _minFreqs(NULL), _maxFreqs(NULL), _avgFreqs(NULL),
	  _maxAmps(NULL), _frames(NULL), _curFrame(NULL)
{
	_frameAlloc();
	SdifGenInit("");
}

SDIFfile::~SDIFfile(void)
{
	SdifGenKill();
	delete [] _filterPartials;
	delete [] _minFreqs;
	delete [] _maxFreqs;
	delete [] _avgFreqs;
	delete [] _maxAmps;
	for (long i = 0; i < _numFrames; i++)
		delete _frames[i];
	free(_frames);
	delete _curFrame;
}

// Create a dynamic array of frames to use when reading from the SDIF file.
// We have to be able to grow the array of Frame pointers while reading
// an SDIF file, because the SDIF header doesn't say how many frames there are.
// (Even if it did, we might be skipping frames the user isn't interested in,
// which can be specified using an SDIF "selection" appended to the file name.)

int SDIFfile::_frameAlloc(void)
{
	Frame **ptr = NULL;
	long newlen = 0;
	if (_frames == NULL) {
		ptr = (Frame **) calloc(kDefaultFrames, sizeof(Frame *));
		newlen = kDefaultFrames;
	}
	else {
		newlen = _framesLen + kDefaultFrames;
		Frame **newptr = (Frame **) realloc(_frames, newlen * sizeof(Frame *));
		if (newptr) {
			ptr = newptr;
			for (long n = _framesLen; n < newlen; n++)  // zero out new portion
				ptr[n] = NULL;
		}
		else
			return -1;	// can't realloc
	}
	_frames = ptr;
	_framesLen = newlen;
	return 0;
}

int SDIFfile::_checkFrameAlloc(void)
{
	if (_numFrames == _framesLen)
		return _frameAlloc();
	return 0;
}

// Most of this comes from the SDIF Library tutorial, last found here:
// http://sdif.sourceforge.net/extern/tutorial-main.html.

int SDIFfile::readFile(const char *fileName)
{
	SdifSignature mysig = SdifSignatureConst('1', 'T', 'R', 'C');
//	char *sigstr = SdifSignatureToString(mysig);

	SdifFileT *file = SdifFOpen(fileName, eReadFile);
	if (file == NULL)
		return -1;
	SdifFReadGeneralHeader(file);
	SdifFReadAllASCIIChunks(file);

	int eof = 0;
	size_t bytesread = 0;
	while (!eof && SdifFLastError(file) == NULL) {
		bytesread += SdifFReadFrameHeader(file);

		while (!SdifFCurrFrameIsSelected(file)
					|| SdifFCurrSignature(file) != mysig) {
			SdifFSkipFrameData(file);
			eof = SdifFGetSignature(file, &bytesread);
			if (eof == eEof)
				break;
			bytesread += SdifFReadFrameHeader(file);
		}

		if (!eof) {
			_checkFrameAlloc();

			SdifSignature sig = SdifFCurrFrameSignature(file);
			SdifUInt4 nmatrix = SdifFCurrNbMatrix(file);
			SdifUInt4 streamid = SdifFCurrID(file);
			SdifFloat8 time = SdifFCurrTime(file);
			if (nmatrix != 1) {
//FIXME: warn once?
			}

			for (SdifUInt4 m = 0; m < nmatrix; m++) {
				bytesread += SdifFReadMatrixHeader(file);

				if (SdifFCurrMatrixIsSelected(file)) {
					SdifSignature sig = SdifFCurrMatrixSignature(file);
					SdifDataTypeET type = SdifFCurrDataType(file);
					SdifInt4 nrows = SdifFCurrNbRow(file);
					SdifInt4 ncols = SdifFCurrNbCol(file);
					if (ncols < 4) {
//FIXME: warn once
					}
					if (nrows > _maxSimultaneousPartials)
						_maxSimultaneousPartials = nrows;

					_frames[_numFrames] = new Frame(nrows);
					_frames[_numFrames]->time(time);
					// get direct access to partial node array
					PartialNode **partials = _frames[_numFrames]->partials();
					_numFrames++;

					for (SdifInt4 row = 0; row < nrows; row++) {
						bytesread += SdifFReadOneRow(file);
						int id = (int) SdifFCurrOneRowCol(file, 1);
						float freq = (float) SdifFCurrOneRowCol(file, 2);
						float amp = (float) SdifFCurrOneRowCol(file, 3);
						float phase = (float) SdifFCurrOneRowCol(file, 4);
						for (SdifInt4 col = 5; col <= ncols; col++)
							SdifFCurrOneRowCol(file, col);	// ignore any other cols
//printf("[%ld] %f %f %f\n", index, freq, amp, phase);
						partials[row]->set(id, freq, amp, phase);
						if (id > _maxPartialID)
							_maxPartialID = id;
					}
				}
				else
					bytesread += SdifFSkipMatrixData(file);

// ?? This is giving 4 bytes padding when the file doesn't have or need this.
				size_t padding = SdifFPaddingCalculate(file->Stream, bytesread);
//printf("padding=%ld\n", padding);
//				bytesread += SdifFReadPadding(file, padding);
//FIXME: this doesn't work if there is no padding.
//				bytesread += SdifFReadPadding(file,
//									SdifFPaddingCalculate(file->Stream, bytesread));
			}

			eof = (SdifFGetSignature(file, &bytesread) == eEof);
		}
	}

	if (SdifFLastError(file)) {
//FIXME:
		return -1;
	}

	SdifFClose(file);

	// Make a frame object for passing filtered frames to users of SDIFfile.
	_curFrame = new Frame(_maxSimultaneousPartials);

	// Make array that will cache filter decision for each partial.
	const int numTracks = maxPartialID() + 1;
	_filterPartials = new bool [numTracks];
	for (int i = 0; i < numTracks; i++)
		_filterPartials[i] = false;

	_hasFile = true;

	return 0;
}

// Copy any unfiltered partials into our spare frame (_curFrame) to return to
// caller of getFrame().  If what we deliver should be the same as last time,
// don't recopy.
void SDIFfile::_copyPartials(const long targetIndex)
{
	if (targetIndex != _lastFrameIndex || _filterChanged) {
		Frame *target = _frames[targetIndex];
		PartialNode **srcPartials = target->partials();
		PartialNode **destPartials = _curFrame->partials();
		const int numSrcPartials = target->numPartials();
		int numDestPartials = 0;
		for (int i = 0; i < numSrcPartials; i++) {
			PartialNode *srcPartial = srcPartials[i];
			const int id = srcPartial->id();
			if (!_filterPartials[id]) {	// then copy remaining partials
				destPartials[numDestPartials]->set(id, srcPartial->freq(),
					srcPartial->amp(), srcPartial->phase());
				numDestPartials++;
			}
		}
		_curFrame->numPartials(numDestPartials);
		_curFrame->time(target->time());
		_filterChanged = false;
	}
}

// Derive a frame with unfiltered partials interpolated between frames located
// at the two given indices. Copy the derived frame into our spare frame
// (_curFrame) to return to caller of getFrame(). The current interpolation
// method is linear. The two frames are assumed to be in ascending time order,
// and requestedTime must be in range [frame1time, frame2time].
//
// Note that we take indices, rather than frames, as arguments to emphasize
// that the two frames must come from the same analysis file, where
// corresponding partial IDs actually means something. To interpolate between
// partials from different analysis files requires pairing up partials based on
// their frequencies, rather than IDs.

void SDIFfile::_copyInterpolatePartials(const double requestedTime,
		const long targetIndex1, const long targetIndex2)
{
	Frame *target1 = _frames[targetIndex1];
	if (targetIndex1 == targetIndex2
			|| _equalDouble(requestedTime, target1->time())) {
		_copyPartials(targetIndex1);
		return;
	}
	Frame *target2 = _frames[targetIndex2];
	if (_equalDouble(requestedTime, target2->time())) {
		_copyPartials(targetIndex2);
		return;
	}
	PartialNode **srcPartials1 = target1->partials();
	PartialNode **srcPartials2 = target2->partials();
	PartialNode **destPartials = _curFrame->partials();
	const int numSrcPartials1 = target1->numPartials();
	const int numSrcPartials2 = target2->numPartials();
	int numDestPartials = 0;
	double frac = (requestedTime - target1->time()) / (target2->time() - target1->time());

	// Keep track of unfiltered IDs shared between the two frames.
	const int commonIDsCapacity = numSrcPartials1 + numSrcPartials2;
	int commonIDs[commonIDsCapacity];
	for (int i = 0; i < commonIDsCapacity; i++)
		commonIDs[i] = -1;
	int numCommonIDs = 0;

	// Search frame 1 for unfiltered partials. If one with the same ID is also
	// in frame 2, interpolate between them and store in destination list. If no
	// match found, store the frame 1 partial, with amp interpolated from zero.
	for (int i = 0; i < numSrcPartials1; i++) {
		PartialNode *partial1 = srcPartials1[i];
		const int id = partial1->id();
		if (!_filterPartials[id]) {
			PartialNode *partial2 = target2->partialFromID(id);
			if (partial2) {
				// interpolate two partial breakpoints and store result
				const float freq = _interp(frac, partial1->freq(), partial2->freq());
				const float amp = _interp(frac, partial1->amp(), partial2->amp());
//FIXME: how to interpolate phase?
				const float phase = partial1->phase();
				destPartials[numDestPartials++]->set(id, freq, amp, phase);
				commonIDs[numCommonIDs++] = id;
			}
			else {
				// use only partial1 breakpoint
				const float amp = _interp(frac, partial1->amp(), 0.0);
				destPartials[numDestPartials++]->set(id, partial1->freq(), amp, partial1->phase());
			}
		}
	}

	// Include any eligible partials from target2 that haven't been used.
	for (int i = 0; i < numSrcPartials2; i++) {
		PartialNode *partial2 = srcPartials2[i];
		const int id = partial2->id();
		if (!_filterPartials[id]) {
			bool found = false;
			for (int j = 0; j < numCommonIDs; j++) {
				if (id == commonIDs[j]) {
					found = true;
					break;
				}
			}
			if (!found) {
				// use only partial2 breakpoint
				const float amp = _interp(frac, 0.0, partial2->amp());
				if (numDestPartials < _maxSimultaneousPartials) {
					destPartials[numDestPartials++]->set(id, partial2->freq(), amp, partial2->phase());
				}
			}
		}
	}

	_curFrame->numPartials(numDestPartials);
	_curFrame->time(requestedTime);
	_filterChanged = false;
}

// Return a frame of partials from the SDIF file, possibly filtered,
// that is close to the requested time, with proximity specified by <criterion>.
// NOTE: Do *not* assume that two frames returned from getFrame have the
// same content if the Frame pointer is the same! Check the frame time instead.
// Even then, they could be filtered differently, if the filtering criteria
// have been changed by filterPartials().

//FIXME: Since the frames are sorted by time, we might be able to improve
// search efficiency by deriving an index from the time value, though SDIF
// files can have weird things like two frames that have the same time, 
// even though that is not supposed to happen.
// One possibility is to somehow hash the times so that they can function
// as array indices. (Might be okay for our purposes, despite this:
// http://stackoverflow.com/questions/4238122/hash-function-for-floats.)
// It might also be better to store the frames in a b-tree, to get
// better search performance.
// However: it's not clear that the current setup is much of a bottleneck
// for the kinds of files that are typical (e.g., 30 seconds). Check this
// with the reasssigned time files, which have many more frames.

//FIXME: I reorganized this code so that it would never return the original
//frame, but what if someone wants to do the filtering in the SDIF beforehand,
//so as to increase runtime performance? I'm not letting them get that.

Frame *SDIFfile::getFrame(const double time, SelectionCriterion criterion)
{
	long targetIndex = -1;

// example frametimes: 0 1 2 3 4 5 6 7 8 9; time=2.5
	if (criterion == NearestEarlier) {
//FIXME: optimize for calls with last frame time, barring interpolation
		for (long i = 0; i < _numFrames; i++) {
			if (time <= _frames[i]->time()) {
				targetIndex = i - 1;
				if (targetIndex < 0)
					targetIndex = 0;
				break;
			}
		}
		if (targetIndex < 0)
			targetIndex = _numFrames - 1;
		_copyPartials(targetIndex);
	}
	else if (criterion == NearestLater) {
		for (long i = _numFrames - 1; i >= 0; i--) {
			if (time >= _frames[i]->time()) {
				targetIndex = i + 1;
				if (targetIndex >= _numFrames)
					targetIndex = _numFrames - 1;
				break;
			}
		}
		if (targetIndex < 0)
			targetIndex = 0;
		_copyPartials(targetIndex);
	}
	else if (criterion == Interpolated) {
		// find nearest earlier frame
		for (long i = 0; i < _numFrames; i++) {
			if (time <= _frames[i]->time()) {
				targetIndex = i - 1;
				if (targetIndex < 0)
					targetIndex = 0;
				break;
			}
		}
		if (targetIndex < 0)
			targetIndex = _numFrames - 1;
		int targetIndexNext = targetIndex + 1;
		if (targetIndexNext >= _numFrames)
			targetIndexNext = _numFrames - 1;
		_copyInterpolatePartials(time, targetIndex, targetIndexNext);
	}
	else {
		fprintf(stderr, "SDIFfile::getFrame: bad SelectionCriterion\n");
		return NULL;
	}

	_lastFrameIndex = targetIndex;
	return _curFrame;
}

// For each SDIFfile object, sort all frames in ascending order of average
// frequency.  Assumes readFile() and analyze() have been called on each file.
// Also assumes that _avgFreqs[] in SDIFfile is sized to accommodate any
// partial ID number as index, which will be true if analyze has been called.
void SDIFfile::prepareForMorphing(SDIFfile *sdif1, SDIFfile *sdif2)
{
	sdif1->sortFramePartialsByFrequency();
	sdif2->sortFramePartialsByFrequency();
}

Frame *SDIFfile::emptyFrame(void)
{
	_curFrame->numPartials(0);
	_curFrame->time(0.0);
	return _curFrame;
}

// Analyze SDIF file for the frequency range, average frequency, and maximum
// amplitude of each partial track. Can be called multiple times, though it's
// not clear why you would ever do that.
void SDIFfile::analyze(void)
{
	int numTracks = maxPartialID() + 1;
	int nodecounts[numTracks]; // how many nodes in each partial track?
	delete [] _minFreqs;
	_minFreqs = new float [numTracks];
	delete [] _maxFreqs;
	_maxFreqs = new float [numTracks];
	delete [] _avgFreqs;
	_avgFreqs = new float [numTracks];
	delete [] _maxAmps;
	_maxAmps = new float [numTracks];
	for (int i = 0; i < numTracks; i++) {
		_minFreqs[i] = FLT_MAX;
		_maxFreqs[i] = 0.0;
		_avgFreqs[i] = 0.0;
		_maxAmps[i] = 0.0;
		nodecounts[i] = 0;
	}

	for (long i = 0; i < _numFrames; i++) {
		PartialNode **partials = _frames[i]->partials();
		int numPartials = _frames[i]->numPartials();
		for (int j = 0; j < numPartials; j++) {
			int id = partials[j]->id();
			float freq = partials[j]->freq();
			float amp = partials[j]->amp();
			if (freq > _maxFreqs[id])
				_maxFreqs[id] = freq;
			else if (freq < _minFreqs[id])
				_minFreqs[id] = freq;
			_avgFreqs[id] += freq;
			if (amp > _maxAmps[id])
				_maxAmps[id] = amp;
			nodecounts[id]++;
		}
	}

	for (int i = 0; i < numTracks; i++) {
		// A partial track can comprise just a single node, in which case
		// the min freq will still be FLT_MAX.
		if (_minFreqs[i] == FLT_MAX)
			_minFreqs[i] = _maxFreqs[i];
		if (nodecounts[i] > 0) {
//FIXME: Not sure there's a need to maintain both avgFreqs and avgLinOct
			_avgFreqs[i] /= nodecounts[i];
			//_avgFreqsLinOct[i] = _octcps(_avgFreqs[i]);
		}
	}
}

void SDIFfile::filterPartials(const float minFreq, const float maxFreq,
		const float ampThreshold)
{
	if (minFreq != _minFreq || maxFreq != _maxFreq
			|| ampThreshold != _ampThreshold) {
		const int numTracks = maxPartialID() + 1;
		for (int i = 0; i < numTracks; i++) {
			if (_maxAmps[i] < ampThreshold
					|| _minFreqs[i] < minFreq || _maxFreqs[i] > maxFreq) {
				_filterPartials[i] = true;
			}
			else
				_filterPartials[i] = false;
		}
		_minFreq = minFreq;
		_maxFreq = maxFreq;
		_ampThreshold = ampThreshold;
		_filterChanged = true;
	}
#ifdef DEBUG1
	printf("Filtered partials........................\n");
	for (int i = 0; i < numTracks; i++)
		printf("[%d] %d\n", i, _filterPartials[i]);
#endif
}

void SDIFfile::dumpFrames(long firstIndex, long endIndex)
{
	if (endIndex == 0 || endIndex > _maxPartialID)
		endIndex = _maxPartialID;
	for (long i = firstIndex; i < endIndex; i++)
		_frames[i]->dump();
}

