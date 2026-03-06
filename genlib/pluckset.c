extern 	void srrand(unsigned int x);
extern float rrand(void);

void
pluckset(float xlp, float amp, float seed, float c, float *q, float sr)
{
	int len,i;
	float x;
	srrand((unsigned int)seed);
	q[1]=(int)(xlp*sr+4.5);
		len=q[1]-1;
	for(i=4; i<len; i++) {
		x=rrand();
		q[i]=amp;
		if(x < 0.) q[i] = amp;
	}
	q[0]=q[1];
	q[2]=c;
	q[3]=.99-q[2];
}
