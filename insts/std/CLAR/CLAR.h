#define DELSIZE 100

class CLAR : public Instrument {
	int dl1[2],dl2[2],length1,length2;
	float del1[DELSIZE],del2[DELSIZE];
	float amp,namp,dampcoef,oldsig;
	float *amparr,amptabs[2];
	int lenamp;
	float *oamparr,oamptabs[2];
	int olenamp;
	float d2gain, spread;
	int skip;

public:
	CLAR();
	int init(float*, short);
	int run();
	};
