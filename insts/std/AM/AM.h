class AM : public Instrument {
	float amp,*amptable, amptabs[2];
	float *amtable,si,phase;
	int lenam;
	float spread;
	int skip,inchan;

public:
	AM();
	int init(float*, short);
	int run();
	};
