#define NCEES 4

class MULTICOMB : public Instrument {
	float amp,*amptable, amptabs[2];
	float *carray[NCEES],spread[NCEES];
	int skip;

public:
	MULTICOMB();
	virtual ~MULTICOMB();
	int init(float*, short);
	int run();
	};
