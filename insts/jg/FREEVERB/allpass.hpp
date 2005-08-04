// Allpass filter declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _allpass_
#define _allpass_

#if defined(i386)
 #define ANTI_DENORM
#endif

class fv_allpass
{
public:
					fv_allpass();
			void	setbuffer(float *buf, int size);
	inline  float	process(float inp);
			void	mute();
			void	setfeedback(float val);
			float	getfeedback();
// private:
	float	feedback;
	float	antidenorm;
	float	*buffer;
	int		bufsize;
	int		bufidx;
};


// Big to inline - but crucial for speed

inline float fv_allpass::process(float input)
{
	float output;
	float bufout;
	
	bufout = buffer[bufidx];
#ifdef ANTI_DENORM
	bufout += antidenorm;
	antidenorm = -antidenorm;
#endif
	
	output = -input + bufout;
	buffer[bufidx] = input + (bufout*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}

#endif//_allpass

//ends
