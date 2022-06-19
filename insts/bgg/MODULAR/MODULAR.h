#define NSLOTS 7 // # of connections allowed
#define NWAVES 5 // # of waveforms for moscil()
#define TABLELEN 1024 // length of wavetables/envelopes
#define NENVS 5 // # of envelopes for nenv()

class MODULES
{
public:
	virtual float getval()=0;
};


class moscil : public MODULES
{
	Ooscili *theoscil;
	float amp;
	float freq;
	MODULES *freqarray[NSLOTS];
	MODULES *amparray[NSLOTS];
	double wavetable[NWAVES][TABLELEN];

public:
	moscil();
	float getval();
	void setfreq(float);
	void setamp(float);
	void setwave(int);
	int connectfreq(MODULES*);
	void disconnectfreq(int);
	int connectamp(MODULES*);
	void disconnectamp(int);

	// for sigout
	int outslot;
	int ampslot;

	// for oscil
	int freqslot;
	int moscampslot;

	// for mmoogcvf
	int mvcfoutslot;
	int mvcfampslot;
	int resslot;
	int cfslot;

	bool is_calculated;
	float retval;
};


class msigout : public MODULES
{
	MODULES *sigarray[NSLOTS];
	MODULES *amparray[NSLOTS];
	float amp;

public:
	msigout();
	void setamp(float);
	int connect(MODULES*);
	void disconnect(int);
	float getval();
	int connectamp(MODULES*);
	void disconnectamp(int);
};


class menv : public MODULES
{
	double envtable[NENVS][TABLELEN];
	float curval;
	double index;
	double incr;
	int is_looping;

public:
	menv();
	int connect(MODULES*);
	void disconnect(int);
	void ping(float);
	void settimelength(float);
	void setloop(int);
	float getval();

	int ampslot;
	float overamp;
	int freqslot;
	int outslot;

	// for moogvcf
	int cfslot;
	int resonslot;
	int mvcfampslot;

	int curenv;
	double timelength;
	bool is_going;
	bool is_calculated;
	float retval;
};


class mmoogvcf : public MODULES
{
	MODULES *sigarray[NSLOTS];
	MODULES *amparray[NSLOTS];
	MODULES *resarray[NSLOTS];
	MODULES *cfarray[NSLOTS];
	float amp;
	float resonance;
	float centerfreq;
	float    f, p, q, b0, b1, b2, b3, b4;

public:
	mmoogvcf();
	void setamp(float);
	void setres(float);
	void setcfreq(float);
	int connect(MODULES*);
	void disconnect(int);
	int connectamp(MODULES*);
	void disconnectamp(int);
	int connectres(MODULES*);
	void disconnectres(int);
	int connectcf(MODULES*);
	void disconnectcf(int);
	float getval();

	int outslot;
	int mvcfoutslot;
	bool is_calculated;
	float retval;
};


class MODULAR : public Instrument {
	int branch;

	void doupdate();
	
	msigout *outsig;
	moscil *oscils[NSLOTS];
	menv *envs[NSLOTS];
	mmoogvcf *moogvcfs[NSLOTS];

	int fd;
	long oldtime;
	int nbytes, totbytes;

public:
	MODULAR();
	virtual ~MODULAR();
	virtual int init(double p[], int n_args);
	virtual int run();
};

