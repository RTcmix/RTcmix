#include <sys/types.h>

class Instrument {
public:
	float start;
	float dur;
	int cursamp;
	int chunksamps;
	int endsamp;
	int nsamps;
	unsigned long chunkstart;

	int sfile_on; 		// a soundfile is open (for closing later)
	int fdIndex;            // index into unix input file desc. table
	int inputchans;
	double inputsr;
	off_t fileOffset;       // current offset in file for this inst
	int mytag;              // for note tagging/rtupdate()	

	Instrument();
	virtual ~Instrument();
	virtual int init(float*, short);
	virtual int run();

	virtual float getstart();
	virtual float getdur(); 
	virtual int getendsamp();
	virtual void setendsamp(int);
	virtual void setchunk(int);
	virtual void setchunkstart(int);
private:
	void gone(); // decrements reference to input soundfile
};

extern void heapify(Instrument *Iptr);
extern void heapSched(Instrument *Iptr);
extern int rtsetoutput(float, float, Instrument*);
extern int rtsetinput(float, Instrument*);
extern int rtaddout(float*);
extern int rtbaddout(float*, int);
extern int rtgetin(float*, Instrument*, int);
extern float rtupdate(int, int); // tag, p-field for return value
