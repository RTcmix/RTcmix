#define NSLOTS 7 // # of connections allowed to each input
#define NWAVES 5 // # of waveforms for moscil()
#define TABLELEN 1024 // length of wavetables/envelopes
#define NENVS 5 // # of envelopes for menv()

#define MAXTOKS 10 // # of tokens in a single parse-line
#define MAXLABEL 80 // max number of chars in the token name


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
	void connect(MODULES*, char*);
	void disconnect(MODULES*, char*);

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
	float getval();
	void connect(MODULES*, char*);
	void disconnect(MODULES*, char*);
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
	void ping(float);
	void settimelength(float);
	void setloop(int);
	float getval();

	float overamp;

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
	void connect(MODULES*, char*);
	void disconnect(MODULES*, char*);
	float getval();

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

