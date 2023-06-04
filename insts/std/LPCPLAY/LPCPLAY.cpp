/* LPCPLAY - voice resynthesis instrument

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = pitch factor
   p4 = starting LPC frame
   p5 = ending LPC frame

   // Optional
   p6 = warp		[0 == none]
   p7 = reson cf	[0 == none]
   p8 = reson bw

*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <rt.h>
#include <rtdefs.h>

#include "LPCPLAY.h"
#include "DataSet.h"
#include "setup.h"

static const float kDefaultFrequency = 256.0;

static const int LPCPLAY_amp = 2;
static const int LPCPLAY_pitch = 3;
static const int LPCPLAY_firstframe = 4;
static const int LPCPLAY_lastframe = 5;
static const int LPCPLAY_warp = 6;
static const int LPCPLAY_cf = 7;
static const int LPCPLAY_bw = 8;

inline int min(int x, int y) { return (x < y) ? x : y; }

#undef debug
#undef debug2   /* super detailed */

#define OLD_VOICED_CHECK     /* code from pre April 2023 */

LPCINST::WarpFilter::WarpFilter() : _outold(0.0f)
{
	for (int i=0; i<MAXPOLES*2; i++) _past[i] = 0;
}

float
LPCINST::WarpFilter::set(float d, float *c, int npoles)     
{
	_npoles = npoles;			// Store this
	
    for (int m=1; m<npoles; m++) {
		c[m] += d * c[m-1];
    }
    double cl = 1./(1. - d * c[npoles-1]);
    _cq = cl * (1. - d * d);	// Store this magic number :-)
    return cl;
}

void 
LPCINST::WarpFilter::run(float *sig, float d, float * const c, float *out, int outlen)
{
	float *past = _past;	// local copies of class state for efficiency
	int npoles = _npoles;
	float cq = _cq;
	float outold = _outold;
	const int npolem1 = npoles - 1;
	
    for (int i=0; i<outlen; ++i) {
		int n;
        float temp1 = past[npolem1];
        past[npolem1] = cq * outold - d * past[npolem1];
        for (n = npoles-2; n>=0; --n) {
            float temp2 = past[n];
            past[n] = d * (past[n+1] - past[n]) + temp1;
            temp1 = temp2;
        }
        for (n=0; n<npoles; n++) {
            *sig += c[n] * past[n];
        }
        *out++ = outold = *sig++;
    }
	_outold = outold;
}

inline bool inRange(double val, double low, double hi) {
    return val >= low && val <= hi;
}

static void fixOctaves(double *pchvals, int frameCount, int framesToSkip, float targetFrequency)
{
    double prevPch = targetFrequency;
    for (int fr = framesToSkip; fr < frameCount; ++fr) {
        double pch = pchvals[fr];
        printf("fixOctaves: pch[%d]: %.1f ", fr, pch);
        // Find overshot pitches an octave too high
        if (inRange(pch, prevPch * 1.9, prevPch * 2.1)) {
            pchvals[fr] = pch * 0.5;
            printf("-> down octave to %.1f\n", pchvals[fr]);
            prevPch = pchvals[fr];
        }
        // Find undershot pitches an octave too low
        else if (inRange(pch, prevPch * 0.4, prevPch * 0.6)) {
            pchvals[fr] = pch * 2.0;
            printf("-> up octave to %.1f\n", pchvals[fr]);
            prevPch = pchvals[fr];
        }
        else {
            printf("prev pitch %.1f so no change\n", prevPch);
            // Don't include this pitch in the average if it is not in the target range
            if (targetFrequency == 0.0 || inRange(pch, targetFrequency * 0.9, targetFrequency * 1.1)) {
                prevPch = pch;
            }
        }
    }
}

static void fixGaps(double *pchvals, int frameCount, int framesToSkip, float targetFrequency)
{
    double prevPch = targetFrequency;
    double variance = 0.9;
    int count = 0;
    for (int fr = framesToSkip; fr < frameCount; ++fr) {
        double pch = pchvals[fr];
        printf("fixGaps: pch[%d]: %.1f ", fr, pch);
        // Well-behaved pitch value
        if (inRange(pch, prevPch*variance, prevPch/variance)) {
            ++count;
            printf("%d non-gap values\n", count);
            prevPch = pch;
        }
        // Pitch has changed too much - search forward for frame in the same range
        else if (count >= 3) {
            printf("gap value after %d others\n", count);
            count = 0;
            for (int future=fr; future<frameCount; ++future) {
                double fpch = pchvals[future];
                if (inRange(fpch, prevPch*variance, prevPch/variance)) {
                    printf("\tfound target bridging value %.1f at index %d\n", fpch, future);
                    for (int n = fr; n <= future; ++n) {
                        pchvals[n] = prevPch + ((double)(n-fr)/(future-fr)) * (fpch - pch);
                    }
                    prevPch = fpch;
                    fr = future;    // jump forward to begin search from here
                    break;
                }
            }
        }
        else {
            printf("\n");
        }
    }
}

static void smooth(double *pchvals, int frameCount, int framesToSkip, float factor)
{
    double past = pchvals[0];
    for (int fr = framesToSkip; fr < frameCount; ++fr) {
        past = ((1.0-factor) * pchvals[fr]) + (factor * past);
        pchvals[fr] = past;
    }
}

extern int GetDataSet(DataSet **);

extern int GetLPCStuff(double *hithresh, double *lowthresh,
					   float *randamp,
					   bool *unvoiced_rate,
					   float *risetime, float *decaytime,
					   float *ampcutoff,
                       float *pSourceDuration);
					   
extern int GetConfiguration(float *maxdev,
							float *perperiod,
							float *hnfactor,
							bool *autocorrect,
                            float *pitch_fix_octave,
                            bool *fix_gaps,
                            float *smoothingFactor);

LPCINST::LPCINST(const char *name)
	: _dataSet(NULL), _alpvals(NULL), _buzvals(NULL), _functionName(name)
{
	_autoCorrect = false;
	_jcount = _counter = 0; 
	_leftOver = 0;
	_savedOffset = 0;
}

LPCINST::~LPCINST()
{
	RefCounted::unref(_dataSet);
	delete [] _alpvals;
	delete [] _buzvals;
}

#ifdef EMBEDDED
extern "C" {
	int LPCprofile();
}
#endif

int LPCINST::init(double p[], int n_args)
{
	int rval;

#ifdef EMBEDDED
	LPCprofile();
#endif

	if (outputchans != 1)
		return die(name(), "Output file must have 1 channel only\n");

	GetDataSet(&_dataSet);
	if (_dataSet == NULL)
		return die(name(), "No open dataset!\n");

	_dataSet->ref();
	_nPoles = _dataSet->getNPoles();

	rval = localInit(p, n_args);
	if (rval == DONT_SCHEDULE)
		return die(name(), "LocalInit failed.");

	// Finish the initialization
	
	for (int i=0; i<_nPoles*2; i++) _past[i] = 0;

#ifdef debug
    printf("LPCINST::init(this=%p): nSamps=%d\n", this, (int)nSamps());
#endif
	return nSamps();
}

int
LPCINST::configure()
{
	SetupArrays((int)_lpcFrames);
	return 0;
}

/* Construct an instance of this instrument and initialize a variable. */
LPCPLAY::LPCPLAY() : LPCINST("LPCPLAY"), _pchvals(NULL), _noisvals(NULL)
{
	_srd2 = SR/2.;
	_magic = 512./SR;
	_phs = 0.0;
	_datafields = LPCPLAY_bw + 1; /* number of fields before pitch curves */
	_voiced = true;		// default state
    _usesFrameTranspositions = false;
    _useTranspositionAsPitch = false;
	for (int i=0; i<9; i++)
		_rsnetc[i]=0;
	_arrayLen = 0;
}


/* Destruct an instance of this instrument. Here's where to free any memory
   you may have allocated.
*/
LPCPLAY::~LPCPLAY()
{
	delete [] _pchvals;
	delete [] _noisvals;
}

/* Called by the scheduler to initialize the instrument. Things done here:
     - read, store and check pfields
     - set output file (or bus) pointer
     - init makegen tables and other instrument-specific things
     - set control rate counter
   If there's an error here (like invalid pfields), call die() to report
   the error and exit. If you just want to warn the user and keep going,
   call warn() with a message.
*/

int LPCPLAY::localInit(double p[], int n_args)
{
   int i;

	if (!n_args || n_args < 6)
		return die("LPCPLAY",
				   "p[0]=starting time, p[1]=duration, p[2]=amp, p[3]=pitch, p[4]=frame1, p[5]=frame2, [ p[6]=warp, p[7]=resoncf, p[8]=resonbw, [ p9--> pitchcurves ] ]\n");

    double outskip = (float)p[0];
	double duration = (float)p[1];
	_amp = p[LPCPLAY_amp];
	_pitch = p[LPCPLAY_pitch];
    _warpFactor = p[LPCPLAY_warp];    // defaults to 0

	int startLPCFrame = (int) p[LPCPLAY_firstframe];
	int endLPCFrame = (int) p[LPCPLAY_lastframe];
	int lpcFrameCount = endLPCFrame - startLPCFrame + 1;

	if (lpcFrameCount <= 0)
		return die("LPCPLAY", "Ending frame must be > starting frame.");

	_envFun = floc(ENV_SLOT);
	sbrrand(1);

	// Pull all the current configuration information out of the environment.
	
	GetLPCStuff(&_highthresh,
				&_lowthresh,
				&_randamp,
				&_unvoiced_rate,
				&_risetime, 
				&_decaytime,
				&_cutoff,
                &_sourceDuration);
			
    float pitchFixOctave, smoothingFactor;
    bool fixPitchGaps = false;
    
	GetConfiguration(&_maxdev,
					 &_perperiod,
					 &_hnfactor,
					 &_autoCorrect,
                     &pitchFixOctave,
                     &fixPitchGaps,
                     &smoothingFactor);
	
	// Pitch table
	_pchvals = new double[lpcFrameCount];

    // Read the pitch values out of data set into _pchvals array
	_lpcFrames = lpcFrameCount;
	_lpcFrame1 = startLPCFrame;
    int unvoicedCount = 0, initialUnvoicedCount = 0;
    bool firstVoicedFound = false;
	for (i = startLPCFrame; i <= endLPCFrame; ++i) {
		double dindex = i;
		if (_dataSet->getFrame(dindex, _coeffs) < 0)
			break;
        /* This is just in case I am using datasets with no pitch value stored */
		_pchvals[i - startLPCFrame] = (_coeffs[PITCH] != 0.0 ? _coeffs[PITCH] : kDefaultFrequency);
        
        if (_coeffs[THRESH] > _highthresh) {
            ++unvoicedCount;
            // Count initial set of unvoiced - preprocessors will skip these
            if (!firstVoicedFound) {
                ++initialUnvoicedCount;
            }
        }
        else {
            firstVoicedFound = true;
        }
	}
    // This pitch preprocessors operate on just the pitches in frames specified by LPCPLAY.
    if (pitchFixOctave != -1.0) {
        rtcmix_advise("LPCPLAY", "Fixing octaves");
        ::fixOctaves(_pchvals, lpcFrameCount, initialUnvoicedCount, pitchFixOctave);
    }
    if (fixPitchGaps) {
        rtcmix_advise("LPCPLAY", "Fixing pitch gaps");
        ::fixGaps(_pchvals, lpcFrameCount, initialUnvoicedCount, pitchFixOctave);
    }
    if (smoothingFactor != 0.0) {
        rtcmix_advise("LPCPLAY", "Smoothing pitch curve");
       ::smooth(_pchvals, lpcFrameCount, initialUnvoicedCount, smoothingFactor);
    }
    _actualWeight = weight(startLPCFrame, endLPCFrame, (float)_highthresh);
	if (_actualWeight == 0.0)
        _actualWeight = kDefaultFrequency;
    
    const float defaultFrameRate = 220.5;   // DAS changed from 112, which was the original rate in cmix
    // User specified duration, so calculate LPC frame increments
    if (duration > 0.) {
        if (_unvoiced_rate) {
            _unvoicedFrameIncrement = defaultFrameRate/SR;
            // If unvoiced frames move at default rate, remaining voiced frames need to move at an adjusted rate
            // to result in the same total length of output.
            int voicedCount = lpcFrameCount - unvoicedCount;
#ifdef debug
            printf("%d unvoiced frames, %d voiced\n", unvoicedCount, voicedCount);
#endif
            double voicedFrameRate = ((double)voicedCount)/(duration - (unvoicedCount/defaultFrameRate));
            _voicedFrameIncrement = voicedFrameRate/SR;
#ifdef debug
            printf("unvoiced fr incr: %.8f, voiced fr incr: %.8f\n", _unvoicedFrameIncrement, _voicedFrameIncrement);
#endif
        }
        else {
            _unvoicedFrameIncrement = _voicedFrameIncrement = 0.0;  // unused in this mode
        }
    }
    else {
        // User wants default frame rate
        duration = lpcFrameCount/defaultFrameRate;
    }

   /* Tell scheduler when to start this inst. This also stores dur and nsamps.
   */
    if (rtsetoutput((float)outskip, (float)duration, this) == -1)
        return DONT_SCHEDULE;

    // Set up amp envelope if any
    evset(SR, (float)duration, _risetime, _decaytime, ENV_SLOT, _evals);
    
    // Pitch pfield magic - interpret based on range.

	// Transpose relative amount using input with +-0.01 == +-1 semitone
    if (ABS(_pitch) < 1.0) {
		_transposition = pow(2.0,(_pitch/.12));
        rtcmix_advise("LPCPLAY", "p[3] will transpose output by %.1f semitones", _pitch*100.0);
    }
	// Transpose relative amount using input as new center in hz
    else if (_pitch > 20) {
		_transposition = _pitch/_actualWeight;
        rtcmix_advise("LPCPLAY", "p[3] will transpose output by %.2fHz/%.2fHz", _pitch, _actualWeight);
    }
	// Transpose using input as new center in octave pt p.c.
    else if (_pitch > 0) {
		_transposition = cpspch(_pitch)/_actualWeight;
        rtcmix_advise("LPCPLAY", "p[3] will transpose output to %.2f octave pt pc", _pitch);
    }
    else if (_pitch < -20) {
		_transposition = -_pitch;  /* flat pitch in hz */
        rtcmix_advise("LPCPLAY", "p[3] will be used as flat pitch of %.2f Hz", _transposition);
        _useTranspositionAsPitch = true;
    }
    else {  // pitch is <= -1.0 and < -20.0 (negative PCH)
		_transposition = cpspch(-_pitch);  /* flat pitch in octave pt */
        rtcmix_advise("LPCPLAY", "p[3] will be used as flat pitch of %.2f Hz (%.2f pch)", _transposition, -_pitch);
        _useTranspositionAsPitch = true;
    }
    
	if (n_args <= _datafields) {
        if (!_useTranspositionAsPitch) {
            rtcmix_advise("LPCPLAY", "Overall transp factor: %f, weighted av. pitch = %g Hz", _transposition, _actualWeight);
            if (_maxdev != 0.0) {
                readjust(_maxdev,_pchvals,startLPCFrame,endLPCFrame,_highthresh,_actualWeight);
            }
            for (i=startLPCFrame; i<=endLPCFrame; ++i) {
                _pchvals[i - startLPCFrame] *= _transposition;
            }
        }
	}
	else {
        if (((n_args - _datafields) % 2) != 0) {
            return die("LPCPLAY", "Frame/transposition values must be specified in pairs");
        }
		int lastfr=_lpcFrame1;
		double transp, lasttr=_transposition;
        rtcmix_advise("LPCPLAY", "%d frame/transposition pairs", (int)((n_args - _datafields) / 2));
		for (int nn = _datafields; nn < n_args; nn+=2) {
            int pframe = (int)p[nn];
            double ptransp = p[nn+1];
			if (ABS(ptransp) < 1.) {
				transp = pow(2.0,(ptransp/.12));
			}
			else {
				transp = cpspch(ABS(ptransp)) / weight((float)lastfr,(pframe+1.),_highthresh);
			}
#ifdef debug
            printf("frame %d transp: %g\n", pframe, transp);
#endif
			double tranincr=(transp-lasttr)/double(pframe-lastfr);
			transp=lasttr;
			for (i=lastfr; i<pframe; i++) {
				_pchvals[i-_lpcFrame1] *= transp;
				transp += tranincr;
			}
			lastfr = pframe;
			lasttr = transp;
		}
        _usesFrameTranspositions = true;    // need this during run-time
        // Don't do this trailing part if we are not using _transposition as a factor!
		if (_pitch > 0.0 && p[n_args-2] < (float) endLPCFrame) {
			/* if last frame in couplets wasn't last frame in batch,
			   use base value from there to the end */
			for (i=lastfr; i<endLPCFrame; ++i)
				_pchvals[i-startLPCFrame] *= _transposition;
		}
	}
	tableset(SR, getdur(), lpcFrameCount, _tblvals);

	_sineFun = floc(SINE_SLOT);
	_reson_is_on = (p[LPCPLAY_cf] != 0.0) ? true : false;
	_cf_fact = p[LPCPLAY_cf];
	_bw_fact = p[LPCPLAY_bw];
	_lpcFrameno = _lpcFrame1;	/* in case first frame is unvoiced */

	return 0;
}

/* Called by the scheduler for every time slice in which this instrument
   should run. This is where the real work of the instrument is done.
*/
int LPCPLAY::run()
{
	int   n = 0;
#ifdef debug
    printf("\nLPCPLAY::run(this=%p): current frame %d\n", this, currentFrame());
#endif

    const int totalAudioFrames = nSamps();  // only used as denominator of a fraction
    
	// Samples may have been left over from end of previous run's block
	if (_leftOver > 0) {
		int toAdd = min(_leftOver, framesToRun());
#ifdef debug
        printf("   this=%p: using %d/%d leftover samps starting at offset %d\n",
			   this, toAdd, _leftOver, _savedOffset);
#endif
		rtbaddout(&_alpvals[_savedOffset], toAdd);
		increment(toAdd);
		n += toAdd;
		_leftOver -= toAdd;
		_savedOffset += toAdd;
	}
	
	for (; n < framesToRun(); n += _counter) {
		double p[12];
		update(p, 12);
		_amp = p[LPCPLAY_amp];
		_pitch = p[LPCPLAY_pitch];
		_warpFactor = p[LPCPLAY_warp];
		_reson_is_on = (p[LPCPLAY_cf] > 0.0) ? true : false;
		_cf_fact = p[LPCPLAY_cf];
		_bw_fact = p[LPCPLAY_bw];
        
        const int currentAudioFrame = currentFrame();

        if (!_unvoiced_rate) {
            // Walk the LPC frames as a fraction of total audio progress
            const double audioFraction = (double)currentAudioFrame/totalAudioFrames;
            _lpcFrameno = _lpcFrame1 + (audioFraction * _lpcFrames);
        }
        else {
            // Increment LPCdata fractional indices
            if (!_voiced) {
                /* if prev frame was unvoiced and unvoiced is running at regular rate, handle that */
               _lpcFrameno += _unvoicedFrameIncrement*_counter;
            }
            else {
                _lpcFrameno += _voicedFrameIncrement*_counter;
            }
        }
        
        // We lock the data set here only because we dont want reentrant calls from parallel LPCPLAY calls.
        _dataSet->lock();
        if (_dataSet->getFrame(_lpcFrameno,_coeffs) == -1) {
            _amp = 0.0;
            _dataSet->unlock();
			break;
        }
        _dataSet->unlock();

        // If requested, stabilize this frame before using
		if (_autoCorrect)
			stabilize(_coeffs, _nPoles);
        double voicedAmp = getVoicedAmp(_coeffs[THRESH]);
        _voiced = (voicedAmp > _highthresh);
#ifdef debug
        printf("\n\tcurrentAudioFrame: %d _lpcFrameno: %.2f ratio: %.9f\n", currentAudioFrame, _lpcFrameno, _lpcFrameno/currentAudioFrame);
#endif
        // equal power handoff between buzz and noise
        const double amp_pi_over2 = voicedAmp * PI * 0.5;
        double buzamp = sin(amp_pi_over2);
		float noisamp = cos(amp_pi_over2) * _randamp;
        
		_ampmlt = _amp * _coeffs[RESIDAMP];
        // Silence any frame with amp below requested threshold
		if (_coeffs[RMSAMP] < _cutoff)
			_ampmlt = 0;
        
#ifdef debug
        printf("\tERR: %.5f lothresh: %.5f hithresh: %.5f voicedAmp: %.5f voiced: %d %sbuzamp: %.3f noisamp: %.3f\n",
               _coeffs[THRESH], _lowthresh, _highthresh, voicedAmp, _voiced, (_lowthresh < _coeffs[THRESH] && _coeffs[THRESH] < _highthresh) ? "MIX: " : "", buzamp, noisamp);
#endif
        // Get cps for this frame from possibly-modified pitch table.
		float cps = tablei(currentAudioFrame,_pchvals,_tblvals);
		float newcps = cps;
        
        // If instrument is using dynamic pitch field, only allow if they did not specify
        // frame-based pitch information.
        if (!_usesFrameTranspositions) {
            double transpFactor = 1.0;
            if (ABS(_pitch) < 1.0) {
                transpFactor = pow(2.0,(_pitch/.12));
            }
            else if (_pitch > 20) {
                transpFactor = _pitch/_actualWeight;
            }
            else if (_pitch > 0) {
                transpFactor = cpspch(_pitch)/_actualWeight;
            }
            else if (_pitch < -20 && _useTranspositionAsPitch) {
                // If input pitch was specified as -Herz, use as actual pitch
                newcps = -_pitch;  /* flat pitch in hz */
            }
            else if (_useTranspositionAsPitch) {
                // If input pitch was specified as -X.YZ, use as actual pitch
                newcps = cpspch(-_pitch);  /* flat pitch in octave pt */
            }
            if (!_useTranspositionAsPitch && transpFactor != _transposition) {
                double transpAdjust = transpFactor / _transposition;
                newcps *= transpAdjust;
            }
        }
        else {
            if (_useTranspositionAsPitch) {
                newcps = _transposition;
            }
        }
#if defined(debug)
        printf("\tframe %g: _pitch %g, cps %g, _transposition %g, newcps %g\n", _lpcFrameno, _pitch, cps, _transposition, newcps);
#endif
		if (_reson_is_on) {
			/* If _cf_fact is greater than 20, treat as absolute freq.
			   Else treat as factor.
			   If _bw_fact is greater than 20, treat as absolute freq.
			   Else treat as factor (i.e., cf * factor == bw).
			*/
			float cf = (_cf_fact < 20.0) ? _cf_fact * cps : _cf_fact;
			float bw = (_bw_fact < 20.0) ? cf * _bw_fact : _bw_fact;
			rszset(SR, cf, bw, 1., _rsnetc);
#ifdef debug2
			printf("\tcf %g bw %g cps %g\n", cf, bw,cps);
#endif
		}
		if (_warpFactor != 0.0) {
			float warp = (_warpFactor > 1.) ? .0001 : _warpFactor;
			_ampmlt *= _warpPole.set(warp, &_coeffs[4], _nPoles);
		}
		if (_hnfactor < 1.0) {
			buzamp *= _hnfactor;	/* compensate for gain change */
		}
        else {
            buzamp /= _hnfactor;    /* compensate for gain increase */
        }
		float hn = (_hnfactor <= 1.0) ? (int)(_hnfactor*_srd2/newcps)-2 : _hnfactor;
		_counter = int(((float)SR/(newcps * _perperiod) ) * .5);
        int remaining = nSamps() - currentAudioFrame;
		_counter = (_counter > remaining) ? remaining : _counter;
#ifdef debug
        printf("\tfr: %.2f err: %.5f bzamp: %.5f noisamp: %.5f newcps: %g _counter: %d remaining: %d\n",
		 	   _lpcFrameno,_coeffs[THRESH],_ampmlt*buzamp,_ampmlt*noisamp,newcps,_counter,remaining);
#endif
        if (_counter <= 0) {
            if (remaining > 0) {
                rtcmix_warn("LPCPLAY", "Counter <= 0!! Frame pitch: %f, %d remaining audio frames", newcps, remaining);
            }
			break;
        }
		// Catch extreme pitches which generate array overruns
		else if (_counter > _arrayLen) {
			rtcmix_warn("LPCPLAY", "Counter exceeded array size -- limiting.  Frame pitch: %f", newcps);
			_counter = _arrayLen;
		}

        const float si = newcps * _magic;
		bbuzz(_ampmlt*buzamp,si,hn,_sineFun,&_phs,_buzvals,_counter);
#ifdef debug2
		printf("\t _buzvals[0] = %g\n", _buzvals[0]);
#endif
		l_brrand(_ampmlt*noisamp,_noisvals,_counter);	/* TEMPORARY */
#ifdef debug2
		printf("\t _noisvals[0] = %g\n", _noisvals[0]);
#endif
		for (int loc=0; loc<_counter; loc++) {
			_buzvals[loc] += _noisvals[loc];	/* add voiced and unvoiced */
		}
        
        // Run N-pole filter, either warped or normal depending on settings
		if (_warpFactor != 0.0) {
			float warp = (_warpFactor > 1.) ? shift(_coeffs[PITCH],newcps,(float)SR) : _warpFactor;
#ifdef debug2
			printf("\t pch: %f newcps: %f warp: %f\n",_coeffs[PITCH], newcps, warp);
#endif
			/*************
			warp = ABS(warp) > .2 ? SIGN(warp) * .15 : warp;
			***************/
			_warpPole.run(_buzvals, warp, &_coeffs[4], _alpvals, _counter);
		}
		else
		{
			ballpole(_buzvals,&_jcount,_nPoles,_past,&_coeffs[4],_alpvals,_counter);
		}
#ifdef debug2
		{ float maxamp=0; for (int x=0;x<_counter;x++) { if (ABS(_alpvals[x]) > ABS(maxamp)) maxamp = _alpvals[x]; }
			printf("\t maxamp = %.4f\n", maxamp);
		}
#endif
        // Apply optional reson filter
        if (_reson_is_on) {
			bresonz(_alpvals,_rsnetc,_alpvals,_counter);
        }
        
		// Apply envelope last
		float envelope = evp(currentAudioFrame,_envFun,_envFun,_evals);
		bmultf(_alpvals, envelope, _counter);

		int sampsToAdd = min(_counter, framesToRun() - n);

		/* Write this block to the output buffer. */
		rtbaddout(_alpvals, sampsToAdd);
		
		/* Keep track of how many sample frames this instrument has generated. */
		increment(sampsToAdd);
#ifdef debug
        printf("\tend of loop: n = %d\n", n);
#endif
	}
	// Handle case where last synthesized block extended beyond framesToRun()
	if (n > framesToRun()) {
		_leftOver = n - framesToRun();
		_savedOffset = _counter - _leftOver;
#ifdef debug
		printf("saving %d samples left over at offset %d\n", _leftOver, _savedOffset);
#endif
	}
	return framesToRun();
}

void
LPCPLAY::SetupArrays(int frameCount)
{
	// Signal arrays -- size depends on update rate.
	const int siglen = _perperiod < 1.0 ?
		(int) (0.5 + MAXVALS / _perperiod) : MAXVALS;
	_alpvals = new float[siglen];
	_buzvals = new float[siglen];
	_noisvals = new float[siglen];
	_arrayLen = siglen;
}

// Returns voiced amp in range 0.0 to 1.0, based on error

double
LPCPLAY::getVoicedAmp(float err)
{
    double sqerr = ::sqrt((double) err);
    double amp = 1.0 - ((sqerr - _lowthresh) / (_highthresh - _lowthresh));
    amp = (amp < 0.0) ? 0.0 : (amp > 1.0) ? 1.0 : amp;
	return amp;
}

float
LPCPLAY::weight(float frame1, float frame2, float thresh)
{
	float c[MAXPOLES+4];
    float xweight = 0.0;
    float sum = 0.0f;
	for (int i=(int)frame1; i<(int)frame2; i++) {
    	_dataSet->getFrame((double)i, c);
    	if ((c[THRESH] <= thresh) || (thresh < 0.)) {
            xweight += c[RMSAMP];
            sum += (c[PITCH] * c[RMSAMP]);
    	}
	}
	return(xweight > 0.0 ? sum/xweight : 0.0);
}

float 
LPCPLAY::deviation(float frame1, float frame2, float weight, float thresh)
{
	float c[MAXPOLES+4];
	int i,j;
	float diff,xweight,sum,dev;
	xweight = sum = 0.001;
	for (j=0,diff=0,i=(int)frame1; i<(int)frame2; i++) {
		_dataSet->getFrame((double)i, c);
		if ((c[THRESH] <= thresh) || (thresh < 0.)) {
			sum += ((ABS((c[PITCH] - weight))) * c[RMSAMP]);
			xweight +=  c[RMSAMP];
		}
	}
	dev = (xweight != 0.0f) ? sum / xweight : 0.0f;
	rtcmix_advise("LPCPLAY", "Average pitch deviation = %f Hz",dev);
	return(dev);
}

void 
LPCPLAY::adjust(float actdev, float desdev, float actweight, 
				double *pchval, float framefirst, float framelast)
{
	int i,j;
	double x,devfact;
	/* two heuristics here: only shrinking range, and no pitches < 50 hz */
	// Note -- now range may grow.
	/*	devfact = (desdev > actdev) ? 1. : desdev/actdev; */
	devfact =  desdev/actdev;
	for (j=0,i=(int)framefirst; i<=(int)framelast; i++,j++) {
		x = (pchval[j]-actweight) * devfact + actweight;
		pchval[j] = (x > 50) ? x : pchval[j];
	}
}

void 
LPCPLAY::readjust(float maxdev, double *pchval, 
			  	  float firstframe, float lastframe, 
			  	  float thresh, float weight)
{
	float dev;
	dev = deviation(firstframe,lastframe,weight,thresh);
	if (!dev)
		dev=.0001;
	if (maxdev) {
		// If negative, use as factor to multiply orig deviation
		if (maxdev < 0)
			maxdev = dev * -maxdev;
		rtcmix_advise("LPCPLAY", "Adjusting pitch deviation to %f Hz",maxdev);
		adjust(dev,maxdev,weight,pchval,firstframe,lastframe);
	}
}

/* LPCIN - sound filtering instrument
 
 p0 = output start time
 p1 = input file skip time
 p2 = duration
 p3 = amplitude multiplier
 p4 = starting LPC frame
 p5 = ending LPC frame
 
 // Optional
 p6 = input channel
 p7 = warp        [0 == none]
 p8 = reson cf    [0 == none]
 p9 = reson bw
 
 */

static const int LPCIN_amp = 3;
static const int LPCIN_warp = 7;
static const int LPCIN_cf = 8;
static const int LPCIN_bw = 9;

LPCIN::LPCIN() : LPCINST("LPCIN"), _inbuf(NULL), _inChannel(0)
{
}

LPCIN::~LPCIN()
{
	delete [] _inbuf;
}

int LPCIN::localInit(double p[], int n_args)
{
	if (!n_args || n_args < 6 || n_args > 10)
		return die("LPCIN",
				   "p[0]=outskip, p[1]=inskip, p[2]=duration, p[3]=amp, p[4]=frame1, p[5]=frame2 [, p[6]=in_channel p[7]=warp p[8]=resoncf, p[9]=resonbw]\n");

	float outskip = p[0];
	float inskip = p[1];
	float ldur = p[2];
	_amp = p[LPCIN_amp];

	int startLPCFrame = (int) p[4];
	int endLPCFrame = (int) p[5];
	int lpcFrameCount = endLPCFrame - startLPCFrame + 1;

	if (lpcFrameCount <= 0)
		return die("LPCIN", "Ending frame must be > starting frame.");

	_inChannel = (int) p[6];
	if (_inChannel >= inputChannels())
		return die("LPCIN", "Requested channel %d of a %d-channel input file",
				   _inChannel, inputChannels());
					   	
	_warpFactor = p[LPCIN_warp];	// defaults to 0

	_reson_is_on = p[LPCIN_cf] != 0.0 ? true : false;
	_cf_fact = p[LPCIN_cf];
	_bw_fact = p[LPCIN_bw];

	// Pull all the current configuration information out of the environment.
	
	double ddummy1, ddummy2;
	float dummy1, dummy2, dummy3, dummy4, dummy5;
	bool bdummy;

	GetLPCStuff(&ddummy1,
				&ddummy2,
				&dummy2,
				&bdummy,
				&dummy3, 
				&dummy4,
				&_cutoff,
                &dummy1);	// All we use is cutoff here

	GetConfiguration(&dummy1,
					 &dummy2,
					 &dummy3,
					 &_autoCorrect,    // All we use is auto-correct here
                     &dummy4,
                     &bdummy,
                     &dummy5);

	// Duration can be calculated from frame count

	const float defaultFrameRate = 112.0;

	ldur = (ldur > 0.) ? ldur : (lpcFrameCount/defaultFrameRate);

   /* Tell scheduler when to start this inst. 
   */
   if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetoutput(outskip, ldur, this) == -1)
		return DONT_SCHEDULE;

	_lpcFrames = lpcFrameCount;
	_lpcFrame1 = startLPCFrame;
	_lpcFrameno = _lpcFrame1;
	return 0;
}

// Called by LPCINST::configure()

void
LPCIN::SetupArrays(int)
{
	_inbuf = new BUFTYPE[inputChannels() * RTBUFSAMPS];
	_alpvals = new float[MAXVALS];
	_buzvals = new float[MAXVALS];
}

int LPCIN::run()
{
	int   n = 0;
	const int inchans = inputChannels();

	// Samples may have been left over from end of previous run's block
	if (_leftOver > 0)
	{
		int toAdd = min(_leftOver, framesToRun());
#ifdef debug
		printf("using %d leftover samps starting at offset %d\n",
			   _leftOver, _savedOffset);
#endif
		bmultf(&_alpvals[_savedOffset], _ampmlt, toAdd);	// Scale signal
		rtbaddout(&_alpvals[_savedOffset], toAdd);
		increment(toAdd);
		n += toAdd;
		_leftOver -= toAdd;
		_savedOffset += toAdd;
	}
	
	/* framesToRun() returns the number of sample frames -- 1 sample for each
	  channel -- that we have to write during this scheduler time slice.
	*/
	for (; n < framesToRun(); n += _counter) {
		double p[10];
		update(p, LPCIN_bw + 1);
		_amp = p[LPCIN_amp];
		_warpFactor = p[LPCIN_warp];
		_reson_is_on = (p[LPCIN_cf] > 0.0) ? true : false;
		_cf_fact = p[LPCIN_cf];
		_bw_fact = p[LPCIN_bw];

		_lpcFrameno = _lpcFrame1 + ((float)(currentFrame())/nSamps()) * _lpcFrames;

#ifdef debug
        printf("\tthis=%p: getting frame %.1f of %d (%d out of %d signal samps)\n",
			   this, _lpcFrameno, (int)_lpcFrames, currentFrame(), nSamps());
#endif
        // We lock the data set here only because we dont want reentrant calls from parallel LPCIN calls.
        _dataSet->lock();
        if (_dataSet->getFrame(_lpcFrameno,_coeffs) == -1) {
            _amp = 0.0;
            _dataSet->unlock();
			break;
        }
        _dataSet->unlock();

		// If requested, stabilize this frame before using
		if (_autoCorrect)
			stabilize(_coeffs, _nPoles);

		_ampmlt = _amp * _coeffs[RESIDAMP] / 10000.0;	// XXX normalize this!
		float newcps = (_coeffs[PITCH] > 0.0) ? _coeffs[PITCH] : 64.0;

		if (_coeffs[RMSAMP] < _cutoff)
			_ampmlt = 0;
		if (_reson_is_on) {
			/* Treat _cf_fact as absolute freq.
			   If _bw_fact is greater than 20, treat as absolute freq.
			   Else treat as factor (i.e., cf * factor == bw).
			*/
			float cf = _cf_fact;
			float bw = (_bw_fact < 20.0) ? cf * _bw_fact : _bw_fact;
			rszset(SR, cf, bw, 1., _rsnetc);
			/* printf("%f %f %f %f\n",_cf_fact*cps,
				_bw_fact*_cf_fact*cps,_cf_fact,_bw_fact,cps); */
		}
		
		if (_warpFactor != 0.0)
		{
			float warp = (_warpFactor > 1.) ? .0001 : _warpFactor;
			_ampmlt *= _warpPole.set(warp, &_coeffs[4], _nPoles);
		}
		_counter = int(((float)SR/(newcps * /*_perperiod*/ 1.0) ) * .5);
//		_counter = (RTBUFSAMPS < MAXVALS) ? RTBUFSAMPS : MAXVALS;
//		_counter = (_counter > (nSamps() - currentFrame())) ? nSamps() - currentFrame() : _counter;
		_counter = min(_counter, framesToRun() - n);
				
        if (_counter <= 0)
			break;

		rtgetin(_inbuf, this, _counter);
		// Deinterleave input
		for (int from=_inChannel, to=0; to < _counter; from += inchans, ++to)
			_buzvals[to] = _inbuf[from];

#ifdef debug2
		printf("\t _buzvals[0] = %g\n", _buzvals[0]);
#endif
		if (_warpFactor != 0.0) {
//			float warp = (_warpFactor > 1.) ? shift(_coeffs[PITCH],newcps,(float)SR) : _warpFactor;
			float warp = _warpFactor;
#ifdef debug
			printf("\tpch: %f newcps: %f d: %f\n",_coeffs[PITCH], newcps, warp);
#endif
			/*************
			warp = ABS(warp) > .2 ? SIGN(warp) * .15 : warp;
			***************/
			_warpPole.run(_buzvals, warp, &_coeffs[4], _alpvals, _counter);
		}
		else {
			ballpole(_buzvals,&_jcount,_nPoles,_past,&_coeffs[4],_alpvals,_counter);
		}
#ifdef debug2
		{ int x; float maxamp=0; for (x=0;x<_counter;x++) { if (ABS(_alpvals[x]) > ABS(maxamp)) maxamp = _alpvals[x]; }
			printf("\t maxamp = %g\n", maxamp);
		}
#endif
		if (_reson_is_on)
			bresonz(_alpvals,_rsnetc,_alpvals,_counter);

		int sampsToAdd = min(_counter, framesToRun() - n);
		
//		printf("\tscaling %d samples by %g\n", sampsToAdd, _ampmlt);

		bmultf(_alpvals, _ampmlt, sampsToAdd);	// Scale signal
		
		/* Write this block to the output buffer. */
		rtbaddout(_alpvals, sampsToAdd);
		
		/* Keep track of how many sample frames this instrument has generated. */
		increment(sampsToAdd);
	}
	// Handle case where last synthesized block extended beyond framesToRun()
	if (n > framesToRun()) {
		_leftOver = n - framesToRun();
		_savedOffset = _counter - _leftOver;
#ifdef debug
		printf("saving %d samples left over at offset %d\n", _leftOver, _savedOffset);
#endif
	}

	return framesToRun();
}

/* The scheduler calls this to create an instance of this instrument,
   and to set up the bus-routing fields in the base Instrument class.
   This happens for every "note" in a score.
*/
Instrument *makeLPCPLAY()
{
   LPCPLAY *inst;

   inst = new LPCPLAY();
   inst->set_bus_config("LPCPLAY");

   return inst;
}

Instrument *makeLPCIN()
{
   LPCIN *inst;

   inst = new LPCIN();
   inst->set_bus_config("LPCIN");

   return inst;
}

/* The rtprofile introduces the instruments to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
#ifndef EMBEDDED
void rtprofile()
{
   RT_INTRO("LPCPLAY", makeLPCPLAY);
   RT_INTRO("LPCIN", makeLPCIN);
}
#endif

