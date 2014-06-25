// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _comb_
#define _comb_

#if defined(i386)
 #define ANTI_DENORM
#endif

class fv_comb
{
public:
					fv_comb();
			void	setbuffer(float *buf, int size);
	inline  float	process(float inp);
			void	mute();
			void	setdamp(float val);
			float	getdamp();
			void	setfeedback(float val);
			float	getfeedback();
private:
	float	feedback;
	float	filterstore;
	float	damp1;
	float	damp2;
#ifdef ANTI_DENORM
	float	antidenorm;
#endif
	float	*buffer;
	int		bufsize;
	int		bufidx;
};


// Big to inline - but crucial for speed

inline float fv_comb::process(float input)
{
	float output;

	output = buffer[bufidx];
#ifdef ANTI_DENORM
	output += antidenorm;
#endif

	filterstore = (output*damp2) + (filterstore*damp1);
#ifdef ANTI_DENORM
	filterstore += antidenorm;
	antidenorm = -antidenorm;
#endif

	buffer[bufidx] = input + (filterstore*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif //_comb_

//ends
