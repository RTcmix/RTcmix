#include "minc_internal.h"

int yywrap() 
{
	free_symbols();
	return 1;
}
