#include <ugens.h>

float pchmidi(unsigned char midinote)
{
	return (pchoct(((float)midinote/12.) + 3.));
}
