#include <stdlib.h>
#include <ugens.h>

double
m_system(float *p, short n_args, double *pp)
{
	char *command = DOUBLE_TO_STRING(pp[0]);
	return system(command);
}
