#include <ugens.h>

int
profile()
{
	UG_INTRO("oldmatrix",oldmatrix); 
	UG_INTRO("matrix",matrix); 
	UG_INTRO("space",space); 
	UG_INTRO("mikes",mikes); 
	UG_INTRO("mikes_off",mikes_off); 
	UG_INTRO("param",param); 
	UG_INTRO("cparam",cparam); 
	UG_INTRO("path",path); 
	UG_INTRO("cpath",cpath); 
  	UG_INTRO("threshold",threshold); 
  	UG_INTRO("set_attenuation_params",set_attenuation_params); 
  	return 0;
}

static const char *dsoName = "libMMOVE";

int
registerSelf()
{
	registerFunction("MMOVE", dsoName);
	registerFunction("RVB", dsoName);
	registerFunction("oldmatrix", dsoName); 
	registerFunction("matrix", dsoName); 
	registerFunction("space", dsoName); 
	registerFunction("mikes", dsoName); 
	registerFunction("mikes_off", dsoName); 
	registerFunction("param", dsoName); 
	registerFunction("cparam", dsoName); 
	registerFunction("path", dsoName); 
	registerFunction("cpath", dsoName); 
  	registerFunction("threshold", dsoName); 
  	registerFunction("set_attenuation_params", dsoName); 
	return 0;
}
