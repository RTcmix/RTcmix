#include <ugens.h>

void
evset(float SR, float dur, float rise, float dec, int nfrise, float *q)
{
	rise = (rise > 0.) ? rise : -rise * dur;
	dec = (dec > 0.) ? dec : -dec * dur;
	q[0] = dur * SR;
	q[1] = (dur-dec)/dur;
	q[2] = rise/dur;
	q[3] = fsize(nfrise);
	q[4] = dec/dur;
}
