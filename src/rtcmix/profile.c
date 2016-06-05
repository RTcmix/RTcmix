#include <ugens.h>

/* this is for RT instruments to link to in order to resolve the
   profile symbol.  In non-rt instrtuments this function is used
   to place the function on the Minc symbol table */

#ifndef EMBEDDED
int profile() { return 0; }

#else // EMBEDDED
// BGG mm -- this contains all the function profile()'s for max/msp
//		we build a static lib for rtcmix~ and iRTcmix

int
profile()
{
	// from IIR
	UG_INTRO("setup",IIR_setup);

	// from LPCPLAY
	UG_INTRO("lpcstuff",lpcstuff);
	UG_INTRO("dataset",dataset);
	UG_INTRO("freset",freset);
	UG_INTRO("setdev",setdev);
	UG_INTRO("setdevfactor",setdevfactor);
	UG_INTRO("set_thresh",set_thresh);
	UG_INTRO("set_hnfactor",set_hnfactor);
	UG_INTRO("autocorrect",use_autocorrect);

	// from MOVE/PLACE
	UG_INTRO("oldmatrix",oldmatrix);
	UG_INTRO("matrix",matrix);
	UG_INTRO("mikes",mikes);
	UG_INTRO("mikes_off",mikes_off);
	UG_INTRO("param",param);
	UG_INTRO("space",m_space);
	UG_INTRO("cparam",cparam);
	UG_INTRO("path",path);
	UG_INTRO("cpath",cpath);
	UG_INTRO("threshold",threshold);

#ifdef NOT_YET
	// from MMOVE/MPLACE
	UG_INTRO("oldmatrix",m_oldmatrix);
	UG_INTRO("matrix",m_matrix);
	UG_INTRO("mikes",m_mikes);
	UG_INTRO("mikes_off",m_mikes_off);
	UG_INTRO("set_attenuation_params",m_set_attenuation_params);
#endif
	// from PVOC
	UG_INTRO("set_filter", set_filter);

	// from ELL
	UG_INTRO("ellset", ellset);

	// from MROOM
	UG_INTRO("timeset", timeset);

	// from ROOM
	UG_INTRO("roomset", roomset);

	// from joel/jfuncs
	UG_INTRO("sin", m_sin);
	UG_INTRO("cos", m_cos);
	UG_INTRO("lowrand", m_lowrand);
	UG_INTRO("highrand", m_highrand);
	UG_INTRO("trirand", m_trirand);
	UG_INTRO("gaussrand", m_gaussrand);
	UG_INTRO("prob", m_prob);
	UG_INTRO("constrain", m_constrain);
	UG_INTRO("map", m_map);

	// from joel/tuning
	UG_INTRO("diapason", m_diapason);
	UG_INTRO("partch", m_partch);
	UG_INTRO("young", m_young);
	UG_INTRO("just", m_just);
	UG_INTRO("pythag", m_pythag);
	UG_INTRO("meantone", m_mean);
	UG_INTRO("eqtemp", m_eqtemp);
	UG_INTRO("create_scale", m_create_scale);
	UG_INTRO("myscale", m_myscale);

	return 0;
}
#endif // EMBEDDED

