class Ooscili
{
	float si, phase, *array;
	int length;

public:
	Ooscili(float, int);
	float next();
	void setfreq(float);
};
