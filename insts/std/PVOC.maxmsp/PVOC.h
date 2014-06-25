#include <Instrument.h>      /* the base class for this instrument */

class PVFilter;

class PVOC : public Instrument {
public:
	PVOC();
	virtual ~PVOC();
	virtual int init(double	*, int);
	virtual int configure();
	virtual int run();
	
protected:
	BUFTYPE	*_inbuf;         // private interleaved buffer
	int		_inReadOffset, _inWriteOffset;
	int		_cachedInFrames;
	int		_outReadOffset, _outWriteOffset;
	int		_cachedOutFrames;
	int		_first, _valid;
	int		_inputchannel;
	int		_currentsample;
	int		R, N, N2, Nw, Nw2, D, I, i, _in, _on, obank, Np;
	float	_amp;
	float	P, *Hwin, *Wanal, *Wsyn, *_pvInput, *winput;
	float 	*lpcoef, *_fftBuf, *channel, *_pvOutput;
	BUFTYPE	*_outbuf;         // private interleaved buffer
	PVFilter *_pvFilter;
	
	// The following are used by the convert() and unconvert() methods
	float	_fundamental;
	float	_convertFactor, _unconvertFactor;
	float	*_convertPhase, *_unconvertPhase;
	// The following are used by the oscillator bank methods
	float	*_lastAmp, *_lastFreq, *_index, *_table;
	float	_oscThreshold;
	float	_Iinv, _Pinc, _ffac;
	int		_NP;
	
private:
	void	initOscbank(int N, int npoles, int R, int Nw, int I, float P);
	void	oscbank(float C[], int N, float lpcoef[], int npoles,
					int R, int Nw, int I, float P, float O[]);

	int		shiftin(float A[], int N, int D);
	void	convert(float S[], float C[], int N2, int D, int R);
	void	unconvert(float C[], float S[], int N2, int I, int R);
	void	shiftout(float A[], int N, int I, int n);
};
