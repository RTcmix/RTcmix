class SGRANR : public Instrument {
        double *amptable, *wavetable, *grenvtable;
        float tabs[2], tabg[2], si, phase, amp, aamp;
        float freq, dur, rate, ratevar, spread;
        float starttime,evdur,gdur;
        int len,alen,grlen;
	int skip, branch;
	double ratevarlo,ratevarmid,ratevarhi,ratevarti;
	double durlo,durmid,durhi,durti;
	double loclo,locmid,lochi,locti;
	double freqlo,freqmid,freqhi,freqti;
	long granlyrs,grainoverlap,overlapsample,totalgrains,grainsamps;
        float overlapfreq;

public:
	SGRANR();
        int init(double*, int);
        int run();
	double prob(double,double,double,double);
        };
