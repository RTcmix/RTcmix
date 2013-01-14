/* SCRUB - transpose a mono input signal using sync interpolation

   p0 = output start time
   p1 = input start time
   p2 = output duration (time to end if negative)
   p3 = amplitude multiplier
   p4 = transposition factor (literal)
   p5 = sync width in samples
   p6 = sync oversampling
   p7 = input channel [optional, default is 0]
   p8 = percent to left [optional, default is .5]

   Processes only one channel at a time.

   Assumes function table 1 is amplitude curve for the note.
   You can call setline for this.

   SCRUB was written by Doug Scott.  Interpolation code and most of the I/O
   was written by Tobias Kunze-Briseno.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ugens.h>
#include <mixerr.h>
#include "SCRUB.h"
#include <rt.h>
#include <math.h>
#include <string.h>

//#define DEBUG
//#define DEBUG_FULL

#ifdef DEBUG
#define PRINT printf
#else
#define PRINT if (0) printf
#endif

// Set buffer segment size to a power of two to be able to test most
// efficiently for out-of-bounds conditions of the read pointer
// (cf. EnsureRawFramesIdxInbound() below).  An inbound read pointer has
// bit log (gkRingBufSegmentSize << 1) set but bit 
//        2
// log (gkRingBufSegmentSize << 2) cleared.  A pointer to the right segment,
//    2
// on the other hand, has these bits inverted, and a pointer to the left
// segment has both bits cleared.

static const int gkRingBufSegmentSize = 1024;
static const int gkOutOfBoundsTestMask = (gkRingBufSegmentSize << 1 &
					  gkRingBufSegmentSize << 2);
static const int gkOutOfBoundsLeft = 0;
static const int gkOutOfBoundsRight = gkRingBufSegmentSize << 2;

static const int gkNrRawFrames = 3 * gkRingBufSegmentSize;

static const float cpsoct10 = cpsoct(10.0);

SCRUB::SCRUB(int sincWidth, int sincOversampling) : Instrument(),
  kSincWidth(sincWidth), kSincOversampling(sincOversampling),
  pSincTable(NULL), pSincTableDiffs(NULL), fFrameCount(0), fChannels(1),
  pRawFrames(NULL), fCurRawFramesIdx(0.0f), fFileChunkStartFrame(0), fFileChunkEndFrame(0)
{
   _initialized = false;
   _startFrame = 0;
   in = NULL;
   branch = 0;
}


SCRUB::~SCRUB()
{
	delete[] pRawFrames;
	delete[] pSincTable;
	delete[] pSincTableDiffs;
	delete [] in;
}


int SCRUB::init(double p[], int n_args)
{
	float outskip, inskip, dur, dur_to_read;

	if (n_args < 5) {
	  die("SCRUB", "Wrong number of args.");
		return(DONT_SCHEDULE);
	}

	outskip = p[0];
	inskip = p[1];
	dur = p[2];
	amp = p[3];
	speed = p[4];
	if (n_args > 5)
		kSincWidth = (int) p[5];
	if (n_args > 6)
		kSincOversampling = (int) p[6];
	inchan = (n_args > 7) ? (int) p[7] : 0;
	pctleft = (n_args > 8) ? p[8] : 0.5;

	if (dur < 0.0)
	  dur = -dur - inskip;

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (inchan >= inputChannels()) {
		return die("SCRUB", "You asked for channel %d of a %d-channel file.",
				   inchan, inputChannels());
	}
	
	_startFrame = (int) (0.5 + inskip * SR);

	amptable = floc(1);
	if (amptable) {
		int amplen = fsize(1);
		tableset(SR, dur, amplen, tabs);
	}
	else
		rtcmix_advise("SCRUB", "Setting phrase curve to all 1's.");
	aamp = amp;

	skip = (int) (SR / (float) resetval);

	return nSamps();
}

int SCRUB::configure()
{
	in = new float[inputChannels() * RTBUFSAMPS];
	int length = (int)(kSincWidth * kSincOversampling);
	pSincTable = new float[length + 1]; // store the last 0.0, too
	pSincTableDiffs = new float[length];
	if (pSincTable && pSincTableDiffs) {
		MakeSincTable();
	}
	else {
		delete [] in;
		in = NULL;
	}
	pRawFrames = new float[gkNrRawFrames * fChannels];
	return in && pRawFrames ? 0 : -1;
}

int SCRUB::run()
{
	const int frameCount = framesToRun();
	int       i, inChans = inputChannels();
	float     *outp;
	double    frac;

	// HACK
	if (!_initialized) {
		Initialize();
		_initialized = 1;
	}
	
	outp = outbuf;               /* point to inst private out buffer */

	GetFrames(in, frameCount, speed);

	for (i = 0; i < frameCount; i++) {
		if (--branch < 0) {
			double 	p[6];
			update(p, 6);
			amp = p[3];
			speed = p[4];
			if (amptable) {
#ifdef MAXMSP
				aamp = rtcmix_table(currentFrame(), amptable, tabs) * amp;
#else
				aamp = table(currentFrame(), amptable, tabs) * amp;
#endif
			}
			branch = skip;
		}

		double newsig = in[(i * inChans) + inchan];
		outp[0] = newsig * aamp;

		if (outputchans == 2) {
			outp[1] = outp[0] * (1.0 - pctleft);
			outp[0] *= pctleft;
		}

		outp += outputchans;
		increment();
	}

	return i;
}


Instrument *makeSCRUB()
{
   SCRUB *inst;

   inst = new SCRUB();
   inst->set_bus_config("SCRUB");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("SCRUB", makeSCRUB);
}
#endif

inline int max(int x, int y) { return (x >= y) ? x : y; }

int SCRUB::Initialize() {
	fFileChunkStartFrame = _startFrame;
	fFileChunkEndFrame = _startFrame;
	fFrameCount = (speed >= 0.0) ?
		_startFrame + getendsamp() : max(_startFrame, getendsamp());
	PRINT("Initialize: fFrameCount set to %ld\n", fFrameCount);
	// read in initial frames so that fCurRawFramesIdx is centered
	int NPastFrames = gkNrRawFrames / 2;
	int NFutureFrames = gkNrRawFrames - NPastFrames;
	ReadRawFrames(NPastFrames, NFutureFrames);
	fFileChunkStartFrame = _startFrame;
	ReadRawFrames(0, -NPastFrames);
	fCurRawFramesIdx = (float)NPastFrames;
	return 0;
}

//
// ReadRawFrames
// -------------
// Read num frames either forward or backward.  If the file has fewer than
// <nframes> frames, read until the end and reposition the read pointer to
// the beginning of the file until the requested amount of frames has been
// read.  If <nframes> is negative (ie, frames are to be read `backward'),
// recalculate the start position appropriately, then read from there as if
// <nframes> were positive.  
// Since ReadRawFrames() effectively `expands' the range of frames read from
// the file, also adjust fFileChunkStartFrame or fFileChunkEndFrame here,
// depending on whether we read backward or forward (the corresponding other
// value is readjusted in RotateRawFrames(), below, from where ReadRawFrames()
// is called).
// Return 0 on success, -1 on failure.

inline int min(int x, int y) { return (x <= y) ? x : y; }

int SCRUB::ReadRawFrames(int destframe, int nframes) {
  float fromFrame, toFrame;

  if (nframes < 0) {
    fromFrame = (fFileChunkStartFrame + nframes) % fFrameCount;
    if (fromFrame < 0)
      fromFrame += fFrameCount;
    toFrame = fFileChunkEndFrame;
    fFileChunkStartFrame = (int) fromFrame;
  } else {
    fromFrame = fFileChunkEndFrame;
    toFrame = (fFileChunkEndFrame + nframes) % fFrameCount;
    fFileChunkEndFrame = (int)toFrame;
  }

  int res;
  int toread = abs(nframes);
  PRINT("ReadRawFrames calling rtinrepos() with loc = %d\n", (int)fromFrame);
  res = rtinrepos(this, (int) fromFrame, SEEK_SET);
  if (res < 0) {
    return -1;
  }

  int err = 0;
  int read = 0;
  while (1) {
  	PRINT("ReadRawFrames calling rtgetin() for %d frames\n", (toread - read));
    while (read < toread) {
		int framesToRead = min(RTBUFSAMPS, toread - read);
		res = rtgetin(&pRawFrames[(destframe + read) * fChannels],
					  this,
					  framesToRead * fChannels);
		if (res == -1) {
		  err = -1;
		  break;
		}
		else
		  read += res;
	}
    if (read < toread) {
 	  PRINT("ReadRawFrames calling rtinrepos() with loc = 0\n");
      rtinrepos(this, 0, SEEK_SET);
    }
	else
      break;
  }
  return err;
}

//
// RotateRawFrames
// ---------------
// Shift the contents of pRawFrames by frameshift frames (typically
// gkRingBufSegmentSize) left or right and fill the newly freed part with
// fresh frames from the sound file.  A right shift means we are making room
// on the left side, ie, the `past' and occurs during reverse play.  A left
// shift makes room for future samples on the right side, which is the
// normal case in forward play.
// Since rotating the buffer effectively `throws away' frames, readjust the
// file frame pointers, too.

// NOTE:  The directional concepts can be confusing here.  We shift
// pRawFrames *backward* or `left' (ie, <frameshift> is negative) in order to
// make room for new sample frames on the `right' side while reading
// *forward*, and thus call ReadRawFrames() accordingly with a positive
// value for <nframes>.  Conversely, while reading *backward*, we shift
// pRawFrames *forward*, or `right' and then call ReadRawFrames() with a
// negative <nframes>.

void SCRUB::RotateRawFrames(long frameshift) {
  if (frameshift != 0) {
    long byteshift = ((gkNrRawFrames - abs(frameshift))
		      * sizeof(float) * fChannels);
    if (frameshift > 0) {
      memmove(&pRawFrames[frameshift*fChannels], &pRawFrames[0], byteshift);
      fFileChunkEndFrame = (fFileChunkEndFrame - frameshift) % fFrameCount;
      if (fFileChunkEndFrame < 0)
	fFileChunkEndFrame += fFrameCount;
      ReadRawFrames(0, -frameshift);
    } else if (frameshift < 0) {
      memmove(pRawFrames, pRawFrames + -frameshift*fChannels, byteshift);
      fFileChunkStartFrame = (fFileChunkStartFrame - frameshift) % fFrameCount;
      ReadRawFrames(gkNrRawFrames + frameshift, -frameshift);
    }
  }
}

//-----------------------------------------------------------------------------
//
// Sample rate conversion by ideal bandlimited interpolation.
//
// (cf. Julius O. Smith III: "Bandlimited Interpolation -- Introduction and
// Algorithm", 1994 // (http://ccrma-www.stanford.edu/~jos/src/src.html))
//


//
// MakeSincTable
// -------------
// Fill pSincTable with a Hamming-windowed (raised cosine) sinc function up
// to the kSincWidth'th zero-crossing to the left and right.  Since the sinc
// function is symmetric (`even'), we need only store the `right wing'.
// Also fill an auxiliary table (pSincTableDiffs) with differences between
// the sinc samples for efficiency.

void SCRUB::MakeSincTable() {
  int length = (int)(kSincWidth * kSincOversampling) + 1;
  float T_s = (float)M_PI / kSincOversampling; // sinc sample interval
  float T_w = T_s / kSincWidth; // window sample interval
  float rad;

  pSincTable[0] = 1.0;
  for (int i = 1; i < length; i++) {
    rad = i * T_s;
    // scale sinc by a Hamming (raised cosine) window
    pSincTable[i] = sin(rad)/rad * (.5 * cos(i * T_w) + .5);
    pSincTableDiffs[i-1] = pSincTable[i] - pSincTable[i-1];
  }
} 


//
// EnsureRawFramesIdxInbound
// -------------------------
// This test is performed for every sample frame.  The bit-and stuff is just 
// a fast version of a simple 
// 
//   if ((int)fCurRawFramesIdx > gkRingBufSegmentSize &&
//       (int)fCurRawFramesIdx < (gkRingBufSegmentSize << 1))
//
// for all values of gkRingBufSegmentSize that are powers of 2.

inline void SCRUB::EnsureRawFramesIdxInbound() {
  int shift = 0;
  switch ((int)fCurRawFramesIdx & 0xc00) { 
  case 0x000 : shift = gkRingBufSegmentSize; break;
  case 0x800 : shift = -gkRingBufSegmentSize; break;
  }
  if (shift) {
    RotateRawFrames(shift);
    fCurRawFramesIdx += (float)shift;
  }
}


//
// GetFrames
// ---------
// Perform the rate conversion algorithm on data from pRawFrames until
// <frames> is filled.  For each raw sample, we sum kSincWidth samples to
// its left and right sides times a value of the sinc table so that the
// resulting signal x(t) is reconstructed according to 
//
//            W
//            ---
//    x(t) =   >  x(nT) * h(t - nT)    
//            ---
//            n = -W
//
// with 
// 
//    h(t) = min{1, f'/f} * sinc(min{f, f'}t)
//
//    W    = kSincWidth
//    T    = the sampling interval
//    f    = the original sample frequency
//    f'   = the new sample frequency
//
// If we resample downward, the sinc function has to be stretched in the
// time domain in order to shrink the spectrum in the frequency domain,
// (ie, to avoid aliasing in the resampled signal) and scaled by f'/f to
// compensate for the thus increased amount of energy in the passband.  To
// preserve filter quality, however, we have to increase the number of
// samples for each interpolation by f'/f as well.

void SCRUB::GetFrames(float* frames, const int nframes, const float speed) {
	float F2OverF1 = 1.0f / fabs(speed);
	float ascaler = (F2OverF1 >= 1.0 ? 1.0 : F2OverF1); // amplitude scaler
	float tunit = ascaler * kSincOversampling; // sinc table index scaler

	int cursmp;			// integral (in samples) and ...
	float frac;			// ... fractional part of fCurRawFramesIdx
	int sincidx;			// integral and ...
	float sincfrac;		// ... fractional part of the real sinc index
	float sum;			// accumulated output value (per sample)
	long outidx = 0L;		// output array fillpointer
	float tleft, tright, tl, tr;	// left and right (continuous) time indices
	int sampl, sampr;    		// left and right wing sample indices
	int sincwidth = int(kSincWidth / ascaler); // preserve filter quality by
						 // raising number of samps
						 // interpolated

	for (int frm = 0; frm < nframes; frm++) { // for each frame do ...
		EnsureRawFramesIdxInbound(); // rotate ring buffer, if necessary
		cursmp = int(fCurRawFramesIdx);
		frac = fCurRawFramesIdx - cursmp;
		cursmp *= fChannels;
		tleft = frac * tunit;
		tright = tunit - tleft;
		for (int chn = 0; chn < fChannels; chn++) { // for each channel do ...
			sum = 0.0f;
			tl = tleft;
			tr = tright;
			sampl = cursmp;
			sampr = cursmp + fChannels;
			for (int k = 0; k < sincwidth; k++) { // for each k'th neighboring sample
				// Note: interpolation here may be omitted if so desired for
				// performance reasons.  Substitute 
				//
				//    sincidx = int(tX);
				  //    sum += pRawFrames[sampl] * pSincTable[sincidx];
				//
				// for both wings, where tX is either tl or tr

				// left wing
				sincidx = int(tl);
				sincfrac = tl - sincidx;
				sum += pRawFrames[sampl] * (pSincTable[sincidx] +
							  pSincTableDiffs[sincidx] * sincfrac);
				// right wing
				sincidx = int(tr);
				sincfrac = tr - sincidx;
				sum += pRawFrames[sampr] * (pSincTable[sincidx] +
							  pSincTableDiffs[sincidx] * sincfrac);
				tl += tunit;
				tr += tunit;
				sampl -= fChannels;
				sampr += fChannels;
			}
			frames[outidx++] = sum * ascaler;	// could optimize * for upsampling
			cursmp++;		// advance to next channel
		}
		fCurRawFramesIdx += speed;
	}
}
