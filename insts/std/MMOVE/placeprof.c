#include <ugens.h>

/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("oldmatrix", oldmatrix); 
   UG_INTRO("matrix", matrix); 
   UG_INTRO("mikes", mikes); 
   UG_INTRO("mikes_off", mikes_off); 
   UG_INTRO("space", space); 
   UG_INTRO("set_attenuation_params", set_attenuation_params); 
   return 0;
}

static const char *dsoName = "libMPLACE";

int
registerSelf()
{
	registerFunction("MPLACE", dsoName);
	registerFunction("RVB", dsoName);
	registerFunction("matrix", dsoName); 
	registerFunction("space", dsoName); 
	registerFunction("mikes", dsoName); 
	registerFunction("mikes_off", dsoName); 
  	registerFunction("set_attenuation_params", dsoName); 
	return 0;
}
