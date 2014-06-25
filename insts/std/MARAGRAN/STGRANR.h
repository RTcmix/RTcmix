class STGRANR : public Instrument {
        float amp,aamp, *in,inskip;
        double *amptable, *grenvtable;
        float tabs[2], tabg[2];
        float transp, dur, rate, ratevar, spread;
        float starttime,evdur,gdur;
        int len,alen,grlen,incount,get_frame,inframe;
	int skip, branch;
	double ratevarlo,ratevarmid,ratevarhi,ratevarti;
	double durlo,durmid,durhi,durti;
	double loclo,locmid,lochi,locti;
	double transplo,transpmid,transphi,transpti;
	long granlyrs,grainoverlap,overlapsample,totalgrains,grainsamps;
        float overlaptransp;
        float  newsig[2], oldsig[2], oldersig[2];
        double increment, counter;

public:
	STGRANR();
	virtual ~STGRANR();
        int init(double*, int );
        int run();
	double prob(double,double,double,double);
        };
