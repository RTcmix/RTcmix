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
							float *hnfactor);

/* Construct an instance of this instrument and initialize a variable. */
LPCINST::LPCINST(const char *name)
	: _dataSet(NULL), _alpvals(NULL), _buzvals(NULL), _functionName(name)
{
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

int LPCINST::init(float p[], int n_args)
{
	if (outputchans != 1) { 
		die(name(), "Output file must have 1 channel only\n");
	}

	GetDataSet(&_dataSet);
	if (_dataSet == NULL) {
		die("LPCPLAY", "No open dataset!\n");
	}
	_dataSet->ref();
	
	_nPoles = _dataSet->getNPoles();

	localInit(p, n_args);

	// Finish the initialization
	
	for (int i=0; i<_nPoles*2; i++) _past[i] = 0;

	/* NSamps() returns the number of sample
      frames that will be written to output (dur * SR). */
	return NSamps();
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
int LPCPLAY::localInit(float p[], int n_args)
{
   int i;

	if (!n_args || n_args < 6) {
		die("LPCPLAY",
		"p[0]=starting time, p[1]=duration, p[2]=amp, p[3]=pitch, p[4]=frame1, p[5]=frame2, [ p[6]=warp p7=resoncf, p8=resonbw [ p9--> pitchcurves ] ]\n");
	}

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
		die("LPCPLAY", "Ending frame must be > starting frame.");

	_warpFactor = p[6];	// defaults to 0

	// Duration can be calculated from frame count

	const float defaultFrameRate = 112.0;

	ldur = (ldur > 0.) ? ldur : (frameCount/defaultFrameRate);

   /* Tell scheduler when to start this inst. 
   */
	rtsetoutput(outskip, ldur, this);

	_envFun = floc(2);
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
					 &_hnfactor);
	
	SetupArrays(frameCount);

	// Finish the initialization
	
	float *cpoint = _coeffs + 4;
	evset(getdur(), _risetime, _decaytime, 2, _evals);

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

	if ((n_args <= _datafields) && (_pitch > 0)) {
		advise("LPCPLAY", "Overall transp factor: %f, weighted av. pitch = %f",
			   _transposition, actualweight);
		if (_maxdev) 
			readjust(_maxdev,_pchvals,startFrame,endFrame,_thresh,actualweight);
		for (i=startFrame; i<=endFrame; ++i) {
			_pchvals[i - startFrame] *= _transposition;
		}
	}
	else {
		int nn=_datafields-1;
		int lastfr=_frame1;
		float transp, lasttr=_transposition;
		while((nn+=2)<n_args) {
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
	tableset(getdur(), frameCount, _tblvals);
//	actualweight = weight(startFrame,endFrame,_thresh);
	float actualcps = cpspch(ABS(_pitch));
	
	/* note, dont use this feature unless pitch is specified in p[3]*/
	
	_sineFun = (float *)floc(1);
	_reson_is_on = p[7] ? true : false;
	_cf_fact = p[7];
	_bw_fact = p[8];
	_frameno = _frame1;	/* in case first frame is unvoiced */
	return 0;
}

/* Called by the scheduler for every time slice in which this instrument
   should run. This is where the real work of the instrument is done.
*/
int LPCPLAY::run()
{
	int   n = 0;
	float out[2];        /* Space for only 2 output chans! */

	/* You MUST call the base class's run method here. */
	Instrument::run();

	// Samples may have been left over from end of previous run's block
	if (_leftOver > 0)
	{
		int toAdd = min(_leftOver, FramesToRun());
#ifdef debug
		printf("using %d leftover samps starting at offset %d\n",
			   _leftOver, _savedOffset);
#endif
		rtbaddout(&_alpvals[_savedOffset], toAdd);
		n += toAdd;
		_leftOver -= toAdd;
		_savedOffset += toAdd;
	}
	
	/* FramesToRun() returns the number of sample frames -- 1 sample for each
	  channel -- that we have to write during this scheduler time slice.
	*/
	for (; n < FramesToRun(); n += _counter) {
		int loc;
		if ( _unvoiced_rate && !_voiced )
		{
			++_frameno;	/* if unvoiced set to normal rate */
		}
		else
		{
			_frameno = _frame1 + ((float)(CurrentFrame())/nsamps) * _frames;
		}
/*		printf("voiced: %d frame %g\n", _voiced, _frameno); */
		if (_dataSet->getFrame(_frameno,_coeffs) == -1)
			break;
		float buzamp = getVoicedAmp(_coeffs[THRESH]);
		_voiced = (buzamp > 0.1); /* voiced = 0 for 10:1 noise */
		float noisamp = (1.0 - buzamp) * _randamp;	/* for now */
		_ampmlt = _amp * _coeffs[RESIDAMP];
		if (_coeffs[RMSAMP] < _cutoff)
			_ampmlt = 0;
		float cps = tablei(CurrentFrame(),_pchvals,_tblvals);
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
			rszset(cf, bw, 1., _rsnetc);
			/* printf("%f %f %f %f\n",_cf_fact*cps,
				_bw_fact*_cf_fact*cps,_cf_fact,_bw_fact,cps); */
		}
		float si = newpch * _magic;
		_ampmlt *= evp(CurrentFrame(),_envFun,_envFun,_evals);

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
		_counter = (_counter > (nsamps - CurrentFrame())) ? nsamps - CurrentFrame() : _counter;
#ifdef debug
		printf("fr: %g err: %g bzamp: %g noisamp: %g pch: %g ctr: %d\n",
		 	   _frameno,_coeffs[THRESH],_ampmlt*buzamp,_ampmlt*noisamp,newpch,_counter);
#endif
        if (_counter <= 0)
			break;
		// Catch bad pitches which generate array overruns
		else if (_counter > _arrayLen)
			die("LPCPLAY", "Counter exceeds array size.  Frame pitch: %f", newpch);

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

		int sampsToAdd = min(_counter, FramesToRun() - n);

		/* Write this block to the output buffer. */
		rtbaddout(_alpvals, sampsToAdd);
		
		/* Keep track of how many sample frames this instrument has generated. */
		increment(_counter);
	}
	// Handle case where last synthesized block extended beyond FramesToRun()
	if (n > FramesToRun())
	{
		_leftOver = n - FramesToRun();
		_savedOffset = _counter - _leftOver;
#ifdef debug
		printf("saving %d samples left over at offset %d\n", _leftOver, _savedOffset);
#endif
	}

	return FramesToRun();
}

void
LPCPLAY::SetupArrays(int frameCount)
{
	// Pitch table
	_pchvals = new float[frameCount];
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
	advise("LPCPLAY", "Average pitch deviation = %f",dev);
	return(dev);
}

void 
LPCPLAY::adjust(float actdev, float desdev, float actweight, 
				float *pchval, float framefirst, float framelast)
{
	int i,j;
	float x,devfact;
/* two heuristics here: only shrinking range, and no pitches < 50 hz */
/*	devfact = (desdev > actdev) ? 1. : desdev/actdev; */
	devfact =  desdev/actdev;
	for (j=0,i=(int)framefirst; i<=(int)framelast; i++,j++) {
		x = (pchval[j]-actweight) * devfact + actweight;
		pchval[j] = (x > 50) ? x : pchval[j];
	}
}

void 
LPCPLAY::readjust(float maxdev, float *pchval, 
			  	  float firstframe, float lastframe, 
			  	  float thresh, float weight)
{
	float dev;
	dev = deviation(firstframe,lastframe,weight,thresh);
	if (!dev)
		dev=.0001;
	if (maxdev)
		adjust(dev,maxdev,weight,pchval,firstframe,lastframe);
}

LPCIN::LPCIN() : LPCINST("LPCIN")
{
}

LPCIN::~LPCIN()
{
}

int LPCIN::localInit(float p[], int n_args)
{
   int i;

	if (!n_args || n_args < 6 || n_args > 7) {
		die("LPCIN",
		"p[0]=outskip, p[1]=inskip, p[2]=duration, p[3]=amp, p[4]=frame1, p[5]=frame2, [ p[6]=warp ]\n");
	}
	float outskip = p[0];
	float inskip = p[1];
	float ldur = p[2];
	_amp = p[3];

	int startFrame = (int) p[4];
	int endFrame = (int) p[5];
	int frameCount = endFrame - startFrame + 1;

	_warpFactor = p[6];	// defaults to 0

	// Duration can be calculated from frame count

	const float defaultFrameRate = 112.0;

	ldur = (ldur > 0.) ? ldur : (frameCount/defaultFrameRate);

   /* Tell scheduler when to start this inst. 
   */
    rtsetinput(inskip, this);
	rtsetoutput(outskip, ldur, this);

	if (inputchans != 1) { 
		die("LPCIN", "Input file must have 1 channel only\n");
	}
					   	
	SetupArrays(frameCount);

	// Finish the initialization
	
	_frames = frameCount;
	_frame1 = startFrame;
	_frameno = _frame1;
	return 0;
}

void
LPCIN::SetupArrays(int)
{
	_alpvals = new float[MAXVALS];
	_buzvals = new float[MAXVALS];
}

int LPCIN::run()
{
	int   n = 0;
	float out[2];        /* Space for only 2 output chans! */

	/* You MUST call the base class's run method here. */
	Instrument::run();

	// Samples may have been left over from end of previous run's block
	if (_leftOver > 0)
	{
		int toAdd = min(_leftOver, FramesToRun());
#ifdef debug
		printf("using %d leftover samps starting at offset %d\n",
			   _leftOver, _savedOffset);
#endif
		bmultf(&_alpvals[_savedOffset], _ampmlt, toAdd);	// Scale signal
		rtbaddout(&_alpvals[_savedOffset], toAdd);
		n += toAdd;
		_leftOver -= toAdd;
		_savedOffset += toAdd;
	}
	
	/* FramesToRun() returns the number of sample frames -- 1 sample for each
	  channel -- that we have to write during this scheduler time slice.
	*/
	for (; n < FramesToRun(); n += _counter) {
		int loc;
		_frameno = _frame1 + ((float)(CurrentFrame())/nsamps) * _frames;

//		printf("\tgetting frame %g of %d (%d out of %d signal samps)\n",
//			   _frameno, (int)_frames, CurrentFrame(), nsamps);
		if (_dataSet->getFrame(_frameno,_coeffs) == -1)
			break;
		_ampmlt = _amp * _coeffs[RESIDAMP] / 10000.0;	// XXX normalize this!
		float newpch = (_coeffs[PITCH] > 0.0) ? _coeffs[PITCH] : 256.0;

//		if (_coeffs[RMSAMP] < _cutoff)
//			_ampmlt = 0;
// 		if (_reson_is_on) {
// 			/* If _cf_fact is greater than 20, treat as absolute freq.
// 			   Else treat as factor.
// 			   If _bw_fact is greater than 20, treat as absolute freq.
// 			   Else treat as factor (i.e., cf * factor == bw).
// 			*/
// 			float cf = (_cf_fact < 20.0) ? _cf_fact*cps : _cf_fact;
// 			float bw = (_bw_fact < 20.0) ? cf * _bw_fact : _bw_fact;
// 			rszset(cf, bw, 1., _rsnetc);
// 			/* printf("%f %f %f %f\n",_cf_fact*cps,
// 				_bw_fact*_cf_fact*cps,_cf_fact,_bw_fact,cps); */
// 		}

		float *cpoint = _coeffs + 4;
		
		if (_warpFactor != 0.0)
		{
			float warp = (_warpFactor > 1.) ? .0001 : _warpFactor;
			_ampmlt *= _warpPole.set(warp, cpoint, _nPoles);
		}
//		_counter = int(((float)SR/newpch ) * .5);
		_counter = (RTBUFSAMPS < MAXVALS) ? RTBUFSAMPS : MAXVALS;
		_counter = (_counter > (nsamps - CurrentFrame())) ? nsamps - CurrentFrame() : _counter;
				
        if (_counter <= 0)
			break;

		rtgetin(_buzvals, this, _counter);
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
//		if (_reson_is_on)
//			bresonz(_alpvals,_rsnetc,_alpvals,_counter);

		int sampsToAdd = min(_counter, FramesToRun() - n);
		
//		printf("\tscaling %d samples by %g\n", sampsToAdd, _ampmlt);

		bmultf(_alpvals, _ampmlt, sampsToAdd);	// Scale signal
		
		/* Write this block to the output buffer. */
		rtbaddout(_alpvals, sampsToAdd);
		
		/* Keep track of how many sample frames this instrument has generated. */
		increment(_counter);
	}
	// Handle case where last synthesized block extended beyond FramesToRun()
	if (n > FramesToRun())
	{
		_leftOver = n - FramesToRun();
		_savedOffset = _counter - _leftOver;
#ifdef debug
		printf("saving %d samples left over at offset %d\n", _leftOver, _savedOffset);
#endif
	}

	return FramesToRun();
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
void rtprofile()
{
   RT_INTRO("LPCPLAY", makeLPCPLAY);
   RT_INTRO("LPCIN", makeLPCIN);
}


