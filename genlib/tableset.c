#include "../H/ugens.h"

void
tableset(float dur, int size, float *tab)
{
	*tab = (long)(dur * SR  -.9999);
	*(tab+1) = size - 1;
}
