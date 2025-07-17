#include <Ougens.h>
#include <vector>
		  // the base class for this instrument
typedef struct {
	float waveSampInc; 
	float ampSampInc; 
	float currTime; 
	float ampPhase; 
	float endTime; 
	float panR; 
	float panL; 
	bool isplaying;} 
Grain;

class AUDIOBUFFER {
public:
    AUDIOBUFFER(int size);
    ~AUDIOBUFFER();
    double Get(float index);
    int GetHead();
    int GetSize();
	void SetSize(int size);
	int GetMaxSize();
    bool GetFull();
	bool CanRun();
    void Append(double samp);
    void Print();

private:
    bool _full;
    int _head;
	int _size;
    std::vector<double>* _buffer;
};

class STGRAN2 : public Instrument {

public:
	STGRAN2();
	virtual ~STGRAN2();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
	void addgrain();
	double prob(double low,double mid,double high,double tight);
	void resetgrain(Grain* grain);
	void resetgraincounter();
	int calcgrainsrequired();

private:
	int _nargs;
	int branch;

	bool configured;
	AUDIOBUFFER* buffer;
	float* in;
	int inchan;

	double oneover_cpsoct10;
	double transLow;
	double transMid;
	double transHigh;
	double transTight;

	double grainDurLow;
	double grainDurMid;
	double grainDurHigh;
	double grainDurTight;

	double panLow;
	double panMid;
	double panHigh;
	double panTight;

	float amp;

	std::vector<Grain*>* grains;
	int grainLimit;
	int newGrainCounter;

	double grainRateVarLow;
	double grainRateVarMid;
	double grainRateVarHigh;
	double grainRateVarTight;

	double* grainEnv;
	int grainEnvLen;
	void doupdate();
};

