class Ooscili
{
	float si, phase, *array, dur;
	int length;

public:
	Ooscili(float, int);
	Ooscili(float, float*);
	Ooscili(float, float*, int);
	float next();
	float next(int);
	void setfreq(float);
	void setphase(float);
	int getlength();
	float getdur();
};

class Orand
{
	long rand_x;

public:
	Orand();
	Orand(int);
	void seed(int);
	void timeseed();
	float random();
	float rand();
	float range(float, float);
};
