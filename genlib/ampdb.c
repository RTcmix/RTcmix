#include <math.h>

double ampdb(float db)
{
	return(pow(10.,(double)(db/20.)));
}
