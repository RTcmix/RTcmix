#ifndef _RTCMIX_SCRUB_H_
#define _RTCMIX_SCRUB_H_

#include <Instrument.h>
#include <rtdefs.h>


class SCRUB : public Instrument {
public:
	SCRUB(int sincWidth = 4, int sincOversampling = 20);
	virtual ~SCRUB();
	int init(double *, int);
	int configure();
	int run();
protected:
	int    skip, branch, inchan;
	int _startFrame;
	float speed;
	float  amp, aamp, pctleft;
	double *amptable;
	float  *in, tabs[2];
private:
	int Initialize();

	// Return <nframes> frames in the array pointed to by <frames>,
	// sample-rate-converted according to the factor <speed>.
	void GetFrames(float* frames, const int nframes, const float speed);



	// Clear and reset internal data.
	void ResetData();

	// Read <nframes> raw frames from the soundfile into pRawFrames, starting
	// at pRawFrames[<destframe>].  <nframes> may be negative.
	int ReadRawFrames(int destframe, int nframes);

	// Rotate the buffer pRawFrames <frameshift> frames left (negative) or
	// right (positive).
	void RotateRawFrames(long frameshift);

	// Ensure that fCurRawFramesIdx is pointing to the middle segment of
	// pRawFrames at all times, rotating pRawFrames as needed.
	void EnsureRawFramesIdxInbound();

	// Set up pSincTable and pSincTableDiffs.
	void MakeSincTable();
	
	bool _initialized;

	int kSincWidth;		// number of zero crossings on either side
	int kSincOversampling;	// number of samples per zero crossing
	float* pSincTable;		// `right wing' of a sinc function
	float* pSincTableDiffs;	// differences between sinc samples

	long fFrameCount;		// soundfile: - frame count
	int fChannels;		//            - channels

	float* pRawFrames;		// ring buffer for soundfile frames
	float fCurRawFramesIdx;	// (continuous) buffer read position
	long fFileChunkStartFrame;	// start and end frame numbers of the last
	long fFileChunkEndFrame;	// chunk of samples read from the file.
};

#endif	// _RTCMIX_SCRUB_H_
