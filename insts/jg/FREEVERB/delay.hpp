// Delay line implementation
// written by JGG, 3 Feb 2001 - based on allpass.hpp

#ifndef _delay_
#define _delay_

class fv_delay
{
public:
					fv_delay();
			void	setbuffer(float *buf, int size);
	inline  float	process(float inp);
			void	mute();
			void	setdelaysamps(int val);
			int		getdelaysamps();
// private:
	int		delaysamps;
	float	*buffer;
	int		bufsize;
	int		bufidx;
};


inline float fv_delay::process(float input)
{
	float output = buffer[bufidx];
	buffer[bufidx] = input;

	if (++bufidx >= delaysamps)
		bufidx = 0;

	return output;
}

#endif //_delay

