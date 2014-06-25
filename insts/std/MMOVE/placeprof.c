#include <ugens.h>

/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("oldmatrix", m_oldmatrix); 
   UG_INTRO("matrix", m_matrix); 
   UG_INTRO("mikes", m_mikes); 
   UG_INTRO("mikes_off", m_mikes_off); 
   UG_INTRO("space", m_space); 
   UG_INTRO("set_attenuation_params", m_set_attenuation_params); 
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
