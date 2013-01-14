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
#include <mixerr.h>
#include <rt.h>
#include <rtdefs.h>

#include "LPCPLAY.h"         /* declarations for this instrument class */
#include "DataSet.h"
#include "setup.h"

inline int min(int x, int y) { return (x < y) ? x : y; }

#undef debug

LPCINST::WarpFilter::WarpFilter() : _outold(0.0f)
{
	for (int i=0; i<MAXPOLES*2; i++) _past[i] = 0;
}

float
LPCINST::WarpFilter::set(float d, float *c, int npoles)     
{
	_npoles = npoles;			// Store this
	
    for (int m=1; m<npoles; m++)
		c[m] += d * c[m-1];
    float cl = 1./(1. - d * c[npoles-1]);
    _cq = cl * (1. - d * d);	// Store this magic number :-)
    return cl;
}

void 
LPCINST::WarpFilter::run(float *sig, float d, float *c, float *out, int nvals)
{
	float *past = _past;	// local copies of class state for efficiency
	int npoles = _npoles;
	float cq = _cq;
	float outold = _outold;
	const int npolem1 = npoles - 1;
	
    for (int i=0; i<nvals; ++i) {
		int n;
        float temp1 = past[npolem1];
        past[npolem1] = cq * outold - d * past[npolem1];
        for (n = npoles-2; n>=0; --n) {
            float temp2 = past[n];
            past[n] = d * (past[n+1] - past[n]) + temp1;
            temp1 = temp2;
        }
        for (n=0; n<npoles; n++)  *sig += c[n] * past[n];
        *out++ = outold = *sig++;
    }
	_outold = outold;
}

extern int GetDataSet(DataSet **);

extern int GetLPCStuff(double *hithresh, double *lowthresh,
					   float *thresh,
					   float *randamp,
					   bool *unvoiced_rate,
					   float *risetime, float *decaytime,
					   float *ampcutoff);
					   
extern int GetConfiguration(float *maxdev,
							float *perperiod,
							float *hnfactor,
							bool *autocorrect);

/* Construct an instance of this instrument and initialize a variable. */
LPCINST::LPCINST(const char *name)
	: _dataSet(NULL), _alpvals(NULL), _buzvals(NULL), _functionName(name)
{
	_autoCorrect = false;
	_jcount = _counter = 0; 
	_leftOver = 0;
	_savedOffset = 0;
}

/* Destruct an instance of this instrument. Here's where to free any memory
   you may have allocated.
*/
LPCINST::~LPCINST()
{
	_dataSet->unref();
	delete [] _alpvals;
	delete [] _buzvals;
}

#ifdef MAXMSP
extern "C" {
	int LPCprofile();
}
#endif

int LPCINST::init(double p[], int n_args)
{
	int rval;

#ifdef MAXMSP
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

	/* nSamps() returns the number of sample
      frames that will be written to output (dur * SR). */
	return nSamps();
}

int
LPCINST::configure()
{
	SetupArrays((int)_frames);
	return 0;
}

/* Construct an instance of this instrument and initialize a variable. */
LPCPLAY::LPCPLAY()
	: LPCINST("LPCPLAY"), _pchvals(NULL), _noisvals(NULL)
{
	_srd2 = SR/2.;
	_magic = 512./SR;
	_phs = 0.0;
	_datafields = 10; /* number of fields before pitch curves */
	_voiced = true;		// default state
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

/* BGGx
	I need to go back and fix this.  I believe I can just use the class
	variable (presently named "CLASSBRADSSTUPIDUNVOICEDFLAG") but not sure.
	right now I'm just putting in the MAXMSP #ifdefs and don't want
	to refactor this entire tangled mess o' code.

	The issue is that I could NOT get unvoiced frames to work, no matter
	what I did to set the threshold, etc.  I inserted this total hack to
	make it work.  This code is in desparate need of refactoring.
*/
#ifdef MAXMSP
int BRADSSTUPIDUNVOICEDFLAG; // set in setup.cpp (set_thresh())
#endif

int LPCPLAY::localInit(double p[], int n_args)
{
   int i;

	if (!n_args || n_args < 6)
		return die("LPCPLAY",
				   "p[0]=starting time, p[1]=duration, p[2]=amp, p[3]=pitch, p[4]=frame1, p[5]=frame2, [ p[6]=warp p7=resoncf, p8=resonbw [ p9--> pitchcurves ] ]\n");

   /* Store pfields in variables, to allow for easy pfield renumbering.
      You should retain the RTcmix numbering convention for the first
      4 pfields: outskip, inskip, dur, amp; or, for instruments that 
      take no input: outskip, dur, amp.
   */
	float outskip = p[0];
	float ldur = p[1];
	_amp = p[2];
	_pitch = p[3];

	int startFrame = (int) p[4];
	int endFrame = (int) p[5];
	int frameCount = endFrame - startFrame + 1;

	if (frameCount <= 0)
		return die("LPCPLAY", "Ending frame must be > starting frame.");

	_warpFactor = p[6];	// defaults to 0

	// Duration can be calculated from frame count

	const float defaultFrameRate = 112.0;

	ldur = (ldur > 0.) ? ldur : (frameCount/defaultFrameRate);

   /* Tell scheduler when to start this inst. 
   */
	if (rtsetoutput(outskip, ldur, this) == -1)
		return DONT_SCHEDULE;

	_envFun = floc(ENV_SLOT);
	sbrrand(1);

	// Pull all the current configuration information out of the environment.
	
	GetLPCStuff(&_highthresh,
				&_lowthresh,
				&_thresh,
				&_randamp,
				&_unvoiced_rate,
				&_risetime, 
				&_decaytime,
				&_cutoff);
					   
	GetConfiguration(&_maxdev,
					 &_perperiod,
					 &_hnfactor,
					 &_autoCorrect);
	
	// Pitch table
	_pchvals = new double[frameCount];

	// Finish the initialization
	
	float *cpoint = _coeffs + 4;
	evset(SR, getdur(), _risetime, _decaytime, ENV_SLOT, _evals);

	_frames = frameCount;
	_frame1 = startFrame;
	for (i = startFrame; i <= endFrame; ++i) {
		float findex = i;
		if (_dataSet->getFrame(findex, _coeffs) < 0)
			break;
		_pchvals[i - startFrame] = (_coeffs[PITCH] ? _coeffs[PITCH] : 256.);
		/* just in case I am using datasets with no pitch value
			stored */
	}
	float actualweight = weight(startFrame, endFrame, _thresh);
	if (!actualweight)
		actualweight = cpspch(_pitch);
	// Transpose relative amount using input with +-0.01 == +-1 semitone
	if (ABS(_pitch) < 1.0)
		_transposition = pow(2.0,(_pitch/.12));
	// Transpose relative amount using input as new center in hz
	else if (_pitch > 20)
		_transposition = _pitch/actualweight;
	// Transpose using input as new center in octave pt p.c.
	else if (_pitch > 0)
		_transposition = cpspch(_pitch)/actualweight;
	else if (_pitch < -20)
		_transposition = -_pitch;  /* flat pitch in hz */
	else
		_transposition = cpspch(-_pitch);  /* flat pitch in octave pt */

	if (n_args <= _datafields && _pitch > 0) {
		rtcmix_advise("LPCPLAY", "Overall transp factor: %f, weighted av. pitch = %g Hz",
			   _transposition, actualweight);
		if (_maxdev) 
			readjust(_maxdev,_pchvals,startFrame,endFrame,_thresh,actualweight);
		for (i=startFrame; i<=endFrame; ++i) {
			_pchvals[i - startFrame] *= _transposition;
		}
	}
	else {
		int lastfr=_frame1;
		float transp, lasttr=_transposition;
		for (int nn = _datafields-1; nn < n_args; nn+=2) {
			if (ABS(p[nn+1]) < 1.) {
				transp = pow(2.0,(p[nn+1]/.12));
			}
			else {
				transp = cpspch(ABS(p[nn+1])) / 
					weight((float)lastfr,(p[nn]+1.),_thresh);
			}
			float tranincr=(transp-lasttr)/(p[nn]-lastfr);
			transp=lasttr;
			for (i=lastfr;i<(int)p[nn];i++) {
				_pchvals[i-_frame1]*=transp;
				transp+=tranincr;
			}
			lastfr = (int) p[nn];
			lasttr = transp;
		}
		if (p[n_args-2] < (float) endFrame) {
			/* if last frame in couplets wasn't last frame in batch,
			   use base value from there to the end */
			transp = _transposition;
			for (i=lastfr; i<endFrame; ++i)
				_pchvals[i-startFrame] *= transp;
		}
	}
	tableset(SR, getdur(), frameCount, _tblvals);
//	actualweight = weight(startFrame,endFrame,_thresh);
	float actualcps = cpspch(ABS(_pitch));
	
	/* note, dont use this feature unless pitch is specified in p[3]*/
	
	_sineFun = floc(SINE_SLOT);
	_reson_is_on = p[7] ? true : false;
	_cf_fact = p[7];
	_bw_fact = p[8];
	_frameno = _frame1;	/* in case first frame is unvoiced */

#ifdef MAXMSP
// see note above
	CLASSBRADSSTUPIDUNVOICEDFLAG = BRADSSTUPIDUNVOICEDFLAG;
#endif

	return 0;
}

/* Called by the scheduler for every time slice in which this instrument
   should run. This is where the real work of the instrument is done.
*/
int LPCPLAY::run()
{
	int   n = 0;
	float out[2];        /* Space for only 2 output chans! */

#if 0
		printf("\nLPCPLAY::run()\n");
#endif

	// Samples may have been left over from end of previous run's block
	if (_leftOver > 0)
	{
		int toAdd = min(_leftOver, framesToRun());
#ifdef debug
		printf("using %d leftover samps starting at offset %d\n",
			   _leftOver, _savedOffset);
#endif
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
		double p[12];
		update(p, 12);
		_amp = p[2];
		_pitch = p[3];
		_transposition = ABS(_pitch);
		_warpFactor = p[6];
		_reson_is_on = p[7] ? true : false;
		_cf_fact = p[7];
		_bw_fact = p[8];
		
		int loc;
		if ( _unvoiced_rate && !_voiced )
		{
			++_frameno;	/* if unvoiced set to normal rate */
		}
		else
		{
			_frameno = _frame1 + ((float)(currentFrame())/nSamps()) * _frames;
		}
#if 0
		printf("frame %g\n", _frameno);
#endif
		if (_dataSet->getFrame(_frameno,_coeffs) == -1)
			break;
		// If requested, stabilize this frame before using
		if (_autoCorrect)
			stabilize(_coeffs, _nPoles);
			
		float buzamp = getVoicedAmp(_coeffs[THRESH]);
		_voiced = (buzamp > 0.1); /* voiced = 0 for 10:1 noise */
		float noisamp = (1.0 - buzamp) * _randamp;	/* for now */
		_ampmlt = _amp * _coeffs[RESIDAMP];
		if (_coeffs[RMSAMP] < _cutoff)
			_ampmlt = 0;
		float cps = tablei(currentFrame(),_pchvals,_tblvals);
		float newpch = cps;

		// If input pitch was specified as -X.YZ, use this as actual pitch
		if ((_pitch < 0) && (ABS(_pitch) >= 1))
			newpch = _transposition;

		if (_reson_is_on) {
			/* If _cf_fact is greater than 20, treat as absolute freq.
			   Else treat as factor.
			   If _bw_fact is greater than 20, treat as absolute freq.
			   Else treat as factor (i.e., cf * factor == bw).
			*/
			float cf = (_cf_fact < 20.0) ? _cf_fact*cps : _cf_fact;
			float bw = (_bw_fact < 20.0) ? cf * _bw_fact : _bw_fact;
			rszset(SR, cf, bw, 1., _rsnetc);
#ifdef debug
			printf("cf %g bw %g cps %g\n", cf, bw,cps);
#endif
		}
		float si = newpch * _magic;

		float *cpoint = _coeffs + 4;
		
		if (_warpFactor != 0.0)
		{
			float warp = (_warpFactor > 1.) ? .0001 : _warpFactor;
			_ampmlt *= _warpPole.set(warp, cpoint, _nPoles);
		}
		if (_hnfactor < 1.0)
		{
			buzamp *= _hnfactor;	/* compensate for gain increase */
		}
		float hn = (_hnfactor <= 1.0) ? (int)(_hnfactor*_srd2/newpch)-2 : _hnfactor;
		_counter = int(((float)SR/(newpch * _perperiod) ) * .5);
		_counter = (_counter > (nSamps() - currentFrame())) ? nSamps() - currentFrame() : _counter;
#ifdef debug
		printf("fr: %g err: %g bzamp: %g noisamp: %g pch: %g ctr: %d\n",
		 	   _frameno,_coeffs[THRESH],_ampmlt*buzamp,_ampmlt*noisamp,newpch,_counter);
#endif
        if (_counter <= 0)
			break;
		// Catch bad pitches which generate array overruns
		else if (_counter > _arrayLen) {
			rtcmix_warn("LPCPLAY", "Counter exceeded array size -- limiting.  Frame pitch: %f", newpch);
			_counter = _arrayLen;
		}

		bbuzz(_ampmlt*buzamp,si,hn,_sineFun,&_phs,_buzvals,_counter);
#ifdef debug
		printf("\t _buzvals[0] = %g\n", _buzvals[0]);
#endif
		l_brrand(_ampmlt*noisamp,_noisvals,_counter);	/* TEMPORARY */
#ifdef debug
		printf("\t _noisvals[0] = %g\n", _noisvals[0]);
#endif
		for (loc=0; loc<_counter; loc++) {
			_buzvals[loc] += _noisvals[loc];	/* add voiced and unvoiced */
		}
		if (_warpFactor) {
			float warp = (_warpFactor > 1.) ? shift(_coeffs[PITCH],newpch,(float)SR) : _warpFactor;
#ifdef debug
			printf("\tpch: %f newpch: %f d: %f\n",_coeffs[PITCH], newpch, warp);
#endif
			/*************
			warp = ABS(warp) > .2 ? SIGN(warp) * .15 : warp;
			***************/
			_warpPole.run(_buzvals, warp, cpoint, _alpvals, _counter);
		}
		else
		{
			ballpole(_buzvals,&_jcount,_nPoles,_past,cpoint,_alpvals,_counter);
		}
#ifdef debug
		{ int x; float maxamp=0; for (x=0;x<_counter;x++) { if (ABS(_alpvals[x]) > ABS(maxamp)) maxamp = _alpvals[x]; }
			printf("\t maxamp = %g\n", maxamp);
		}
#endif
		if (_reson_is_on)
			bresonz(_alpvals,_rsnetc,_alpvals,_counter);

		// Apply envelope last

		float envelope = evp(currentFrame(),_envFun,_envFun,_evals);
		bmultf(_alpvals, envelope, _counter);

		int sampsToAdd = min(_counter, framesToRun() - n);

		/* Write this block to the output buffer. */
		rtbaddout(_alpvals, sampsToAdd);
		
		/* Keep track of how many sample frames this instrument has generated. */
		increment(sampsToAdd);
	}
	// Handle case where last synthesized block extended beyond framesToRun()
	if (n > framesToRun())
	{
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

double
LPCPLAY::getVoicedAmp(float err)
{
	double sqerr, amp;
	sqerr = ::sqrt((double) err);
	amp = 1.0 - ((sqerr - _lowthresh) / (_highthresh - _lowthresh));
	amp = (amp < 0.0) ? 0.0 : (amp > 1.0) ? 1.0 : amp;

#ifdef MAXMSP
// see note above
	if (CLASSBRADSSTUPIDUNVOICEDFLAG == 1) amp = 0.0;
#endif

	return amp;
}

float 
LPCPLAY::weight(float frame1, float frame2, float thresh)
{
	float c[MAXPOLES+4];
	float xweight,sum;
	xweight = sum = .001;
	for (int i=(int)frame1; i<(int)frame2; i++) {
    	_dataSet->getFrame((float)i, c);
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
		_dataSet->getFrame((float)i, c);
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
	if (maxdev)
	{
		// If negative, use as factor to multiply orig deviation
		if (maxdev < 0)
			maxdev = dev * -maxdev;
		rtcmix_advise("LPCPLAY", "Adjusting pitch deviation to %f Hz",maxdev);
		adjust(dev,maxdev,weight,pchval,firstframe,lastframe);
	}
}

//
// LPCIN
//

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
   int i;

	if (!n_args || n_args < 6 || n_args > 10)
		return die("LPCIN",
				   "p[0]=outskip, p[1]=inskip, p[2]=duration, p[3]=amp, p[4]=frame1, p[5]=frame2 [, p[6]=in_channel p[7]=warp p[8]=resoncf, p[9]=resonbw]\n");

	float outskip = p[0];
	float inskip = p[1];
	float ldur = p[2];
	_amp = p[LPCIN_amp];

	int startFrame = (int) p[4];
	int endFrame = (int) p[5];
	int frameCount = endFrame - startFrame + 1;

	if (frameCount <= 0)
		return die("LPCIN", "Ending frame must be > starting frame.");

	_inChannel = (int) p[6];
	if (_inChannel >= inputChannels())
		return die("LPCIN", "Requested channel %d of a %d-channel input file",
				   _inChannel, inputChannels());
					   	
	_warpFactor = p[LPCIN_warp];	// defaults to 0

	_reson_is_on = p[LPCIN_cf] ? true : false;
	_cf_fact = p[LPCIN_cf];
	_bw_fact = p[LPCIN_bw];

	// Pull all the current configuration information out of the environment.
	
	double ddummy1, ddummy2, ddummy3;
	float dummy1, dummy2, dummy3, dummy4;
	bool bdummy;

	GetLPCStuff(&ddummy1,
				&ddummy1,
				&dummy1,
				&dummy2,
				&bdummy,
				&dummy3, 
				&dummy4,
				&_cutoff);	// All we use is cutoff here

	GetConfiguration(&dummy1,
					 &dummy2,
					 &dummy3,
					 &_autoCorrect);	// All we use is auto-correct here

	// Duration can be calculated from frame count

	const float defaultFrameRate = 112.0;

	ldur = (ldur > 0.) ? ldur : (frameCount/defaultFrameRate);

   /* Tell scheduler when to start this inst. 
   */
   if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetoutput(outskip, ldur, this) == -1)
		return DONT_SCHEDULE;

	_frames = frameCount;
	_frame1 = startFrame;
	_frameno = _frame1;
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
	float out[2];        /* Space for only 2 output chans! */
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
		_reson_is_on = p[LPCIN_cf] ? true : false;
		_cf_fact = p[LPCIN_cf];
		_bw_fact = p[LPCIN_bw];

		int loc;
		_frameno = _frame1 + ((float)(currentFrame())/nSamps()) * _frames;

#ifdef debug
		printf("\tgetting frame %g of %d (%d out of %d signal samps)\n",
			   _frameno, (int)_frames, currentFrame(), nSamps());
#endif
		if (_dataSet->getFrame(_frameno,_coeffs) == -1)
			break;

		// If requested, stabilize this frame before using
		if (_autoCorrect)
			stabilize(_coeffs, _nPoles);

		_ampmlt = _amp * _coeffs[RESIDAMP] / 10000.0;	// XXX normalize this!
		float newpch = (_coeffs[PITCH] > 0.0) ? _coeffs[PITCH] : 64.0;

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

		float *cpoint = _coeffs + 4;
		
		if (_warpFactor != 0.0)
		{
			float warp = (_warpFactor > 1.) ? .0001 : _warpFactor;
			_ampmlt *= _warpPole.set(warp, cpoint, _nPoles);
		}
		_counter = int(((float)SR/(newpch * /*_perperiod*/ 1.0) ) * .5);
//		_counter = (RTBUFSAMPS < MAXVALS) ? RTBUFSAMPS : MAXVALS;
//		_counter = (_counter > (nSamps() - currentFrame())) ? nSamps() - currentFrame() : _counter;
		_counter = min(_counter, framesToRun() - n);
				
        if (_counter <= 0)
			break;

		rtgetin(_inbuf, this, _counter);
		// Deinterleave input
		for (int from=_inChannel, to=0; to < _counter; from += inchans, ++to)
			_buzvals[to] = _inbuf[from];

#ifdef debug
		printf("\t _buzvals[0] = %g\n", _buzvals[0]);
#endif
		if (_warpFactor) {
//			float warp = (_warpFactor > 1.) ? shift(_coeffs[PITCH],newpch,(float)SR) : _warpFactor;
			float warp = _warpFactor;
#ifdef debug
			printf("\tpch: %f newpch: %f d: %f\n",_coeffs[PITCH], newpch, warp);
#endif
			/*************
			warp = ABS(warp) > .2 ? SIGN(warp) * .15 : warp;
			***************/
			_warpPole.run(_buzvals, warp, cpoint, _alpvals, _counter);
		}
		else
		{
			ballpole(_buzvals,&_jcount,_nPoles,_past,cpoint,_alpvals,_counter);
		}
#ifdef debug
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
	if (n > framesToRun())
	{
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

extern Instrument *makeWAVETABLE();

/* The rtprofile introduces the instruments to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("LPCPLAY", makeLPCPLAY);
   RT_INTRO("LPCIN", makeLPCIN);
}
#endif

