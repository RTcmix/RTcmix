class MBLOWBOTL : public Instrument {
	int nargs, branch;
	float   amp, breathamp, pctleft;
	double  *amptable;
	Ooscili *theEnv;
	BlowBotl *theBotl;
	double freq, noiseamp;
	void doupdate();

public:
	MBLOWBOTL();
	virtual ~MBLOWBOTL();
	virtual int init(double *, int);
	virtual int run();
};

// update flags (shift amount is pfield number)
enum {
	kAmp = 1 << 2,
	kFreq = 1 << 3,
	kNoise = 1 << 4,
	kPan = 1 << 6,
	kBreathPress = 1 << 7
};
