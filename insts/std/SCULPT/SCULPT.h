class SCULPT : public Instrument {
	double *wave, *amptable, *freqtable, *pamptable;
	float phase, amp, amptabs[2];
	float spread;
	int len;
	int pdur, pcount, index;

public:
	SCULPT();
	int init(double*, int);
	int run();
	};
