class MBLOWBOTL : public Instrument {
	float   amp, pctleft;
	double  amparray[2];
	Ooscili *theEnv;
	BlowBotl *theBotl;

public:
	MBLOWBOTL();
	virtual ~MBLOWBOTL();
	virtual int init(double *, int);
	virtual int run();
};

