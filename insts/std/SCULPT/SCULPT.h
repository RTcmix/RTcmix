class SCULPT : public Instrument {
	float *wave, phase, amp;
	float *amptable, amptabs[2];
	float *freqtable, *pamptable;
	float spread;
	int len;
	int pdur, pcount, index;

public:
	SCULPT();
	int init(float*, short);
	int run();
	};
