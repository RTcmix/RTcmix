class SCULPT : public Instrument {
	double *wave, *amptable, *freqtable, *pamptable;
	float phase, si, amp, aamp, amptabs[2];
	float spread;
	int len;
	int pdur, branch, index;

public:
	SCULPT();
	virtual int init(double*, int);
	virtual int run();
};
