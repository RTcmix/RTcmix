#include	"../H/ugens.h"

void
ug_intro()
{
/*
 *  This cute macro (from ugens.h) makes the necessary declarations
 *  and adds the function to ug_list
 */
	UG_INTRO("makegen", makegen);
	UG_INTRO("open",m_open);
	UG_INTRO("peakoff",peak_off);
	UG_INTRO("punch",punch_on);
	UG_INTRO("sfclean",m_clean);
	UG_INTRO("sfprint",sfprint);
	UG_INTRO("system",m_system);
	UG_INTRO("sfcopy",sfcopy);
	UG_INTRO("input",m_input);
	UG_INTRO("output",m_output);
	UG_INTRO("cpspch",m_cpspch);
	UG_INTRO("pchcps",m_pchcps);
 	UG_INTRO("pchoct",m_pchoct);
 	UG_INTRO("octpch",m_octpch);
 	UG_INTRO("dbamp",m_dbamp);
	UG_INTRO("octcps",m_octcps);
	UG_INTRO("cpsoct",m_cpsoct);
	UG_INTRO("random",m_random);
	UG_INTRO("rand",m_rand);
	UG_INTRO("srand",m_srand);
	UG_INTRO("fplot",fplot);
	UG_INTRO("fdump",fdump);
	UG_INTRO("tb",m_time_beat);
	UG_INTRO("bt",m_beat_time);
	UG_INTRO("tbase",tbase);
	UG_INTRO("tempo",tempo);
	UG_INTRO("trunc",m_trunc);
	UG_INTRO("ampdb",m_ampdb);
	UG_INTRO("boost",m_boost);
	UG_INTRO("resetamp",resetamp);
	UG_INTRO("sr",m_sr);
	UG_INTRO("chans",m_chans);
	UG_INTRO("class",m_class);
	UG_INTRO("dur",m_dur);
	UG_INTRO("peak",m_peak);
	UG_INTRO("left_peak",m_left);
	UG_INTRO("right_peak",m_right);
	UG_INTRO("load_array",m_load_array);
	UG_INTRO("get_array",m_get_array);
	UG_INTRO("get_sum",m_get_sum);
	UG_INTRO("mod",m_mod);
	UG_INTRO("put_array",m_put_array);
	UG_INTRO("get_size",m_get_size);
	UG_INTRO("max",m_max);
	UG_INTRO("exit",m_exit);
	UG_INTRO("info",m_info);
 	UG_INTRO("infile", m_infile);
 	UG_INTRO("sampfunc",m_sampfunc);
 	UG_INTRO("sampfunci",m_sampfunci);
 	UG_INTRO("getpch",m_getpch);
 	UG_INTRO("getamp",m_getamp);
 	UG_INTRO("stringify",m_stringify);
	UG_INTRO("log",m_log);
	UG_INTRO("pow",m_pow);
	UG_INTRO("round",m_round);
	UG_INTRO("print",m_print);
	UG_INTRO("wrap",m_wrap);
	UG_INTRO("abs",m_abs);
	UG_INTRO("f_arg",f_arg); /* to return float from command line */
	UG_INTRO("i_arg",i_arg); /* to return int from command line */
	UG_INTRO("s_arg",s_arg); /* to return string from command line */
	UG_INTRO("n_arg",n_arg); /* to return num args from command line */
	UG_INTRO("print_on",m_print_is_on); /* to turn on printing*/
	UG_INTRO("print_off",m_print_is_off); /* to turn off printing*/
	UG_INTRO("str_num",str_num); /* string,num,strin,num, etc print out */
	UG_INTRO("get_spray",m_get_spray);
	UG_INTRO("spray_init",m_spray_init);
	UG_INTRO("pchmidi",m_pchmidi);
	UG_INTRO("cpsmidi",m_cpsmidi);
	UG_INTRO("midipch", m_midipch);
	UG_INTRO("rtsetparams",rtsetparams);
	UG_INTRO("rtinput",rtinput);
	UG_INTRO("rtoutput",rtoutput);
	UG_INTRO("set_option",set_option);
	UG_INTRO("setline_size",m_setline_size);
	UG_INTRO("setline",m_setline);
	UG_INTRO("reset",m_reset);
	UG_INTRO("control_rate",m_reset); /* because "reset" is a perlfunc */
	UG_INTRO("load",m_load); /* allows loading of dynamic libraries */
	UG_INTRO("DUR",m_DUR);  /* returns duration for rtinput files */
	UG_INTRO("SR",m_SR);  /* returns rate for rtinput files */
	UG_INTRO("PEAK",m_PEAK);  /* returns peak amp for rtinput files */
	UG_INTRO("LEFT_PEAK",m_LEFT_PEAK);
	UG_INTRO("RIGHT_PEAK",m_RIGHT_PEAK);
	UG_INTRO("bus_config", bus_config);
	UG_INTRO("pickrand", m_pickrand);
	UG_INTRO("pickwrand", m_pickwrand);
	UG_INTRO("irand", m_irand);
	UG_INTRO("setexp", m_setexp);
	UG_INTRO("addgens", m_addgens);
	UG_INTRO("multgens", m_multgens);
#ifdef RTUPDATE
	UG_INTRO("pgen", pgen);
	UG_INTRO("note_pfield_path", note_pfield_path);
	UG_INTRO("inst_pfield_path", inst_pfield_path);
	UG_INTRO("unset_pfield_path", unset_pfield_path);
	UG_INTRO("set_inst_tag_num", set_itag_num);
#endif /* RTUPDATE */
}


