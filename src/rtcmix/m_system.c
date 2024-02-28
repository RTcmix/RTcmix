#include <stdlib.h>
#include <ugens.h>

double
m_system(double *p, int n_args)
{
	char *command = DOUBLE_TO_STRING(p[0]);

#ifdef IOS
	rtcmix_warn("system", "the system command %s not allowed on iDevices", command);
	return (-1.0);
#else
	return system(command);
#endif
}
