
class MYINST : public Instrument {

public:
	MYINST();
	virtual ~MYINST();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	int _nargs, _inchan, _branch;
	float _amp, _pan;
	float *_in;
};

