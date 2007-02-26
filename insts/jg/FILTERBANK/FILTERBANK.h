class Oreson;

class FilterBand {
	Oreson	*_filt;
	float		_cf;
	float		_bw;
	float		_amp;
public:
	FilterBand(float srate, float cf, float bw, float amp);
	~FilterBand();
	inline void setparams(float cf, float bw, float amp) {
		_amp = amp;
		if (cf != _cf || bw != _bw) {
			_cf = cf;
			_bw = bw;
			_filt->setparams(_cf, _bw);
		}
	}
	inline float next(float sig) { return _filt->next(sig) * _amp; }
};


class FILTERBANK : public Instrument {
	int			nargs, skip, branch, insamps, numbands, inchan;
	float			amp, pan;
	float			*in;
	FilterBand	**filt;

	void doupdate();
public:
	FILTERBANK();
	virtual ~FILTERBANK();
	virtual int init(double p[], int n_args);
	virtual int configure();
	virtual int run();
};

