#include <ugens.h>

int
profile()
{
    UG_INTRO("oldmatrix",m_oldmatrix); 
    UG_INTRO("matrix",m_matrix); 
    UG_INTRO("space",m_space); 
    UG_INTRO("mikes",m_mikes); 
    UG_INTRO("mikes_off",m_mikes_off); 
    UG_INTRO("param",param); 
    UG_INTRO("cparam",cparam); 
    UG_INTRO("path",path); 
    UG_INTRO("cpath",cpath); 
    UG_INTRO("threshold",threshold); 
    UG_INTRO("set_attenuation_params",m_set_attenuation_params);
  	return 0;
}

static const char *dsoName = "libMMOVE";

// This function is called, if present, by RTcmix at initialization time
// when it locates and opens the DSO containing this function.  The call
// to registerFunction() takes the string tag for a function that can be
// called from a Minc script, and the name of the DSO containing the function.
// These should all be the same for any given instrument.
//
// At the present time, there is no system to handle multiple attempts to
// register the same function tag.  Only the first attempt will succeed.
//

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
