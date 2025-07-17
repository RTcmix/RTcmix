// BGG wmm
extern "C" {
	extern void post(char *fmt, ...);
}

int wmmcontext_busy;

#include <RTcmix.h>
#include <wmmcontext.h>
// BGGx ww
#include "RTOption.h" // for our print context

extern wmmcontext theContext[]; // in main.cpp

// from gen/makegen.c (now in RTcmix.cpp)
//extern int ngens;
//extern double **farrays;
//extern int *sizeof_farray;
//extern int *f_goto;

// from minc_functions.c
//extern double **minc_arrayw;
//extern double *minc_array_size;

// from minc_functions.c (now declared in RTcmix.cpp)
// BGGxx ww
extern "C" {
extern int resetval;
}

// BGGxx ww
extern "C" { 
// from main.cpp (now in RTcmix.cpp)
extern int bang_ready;

// from main.cpp (now in RTcmix.cpp)
extern int vals_ready;
extern float *maxmsp_vals;
}

// from main.cpp
extern int firstprint;
extern char *pcheckptr;

// from main.cpp
//extern float *inletvals;

// from main.cpp (now in RTcmix.cpp)
// BGGx
//extern mm_buf *mm_bufs;
//extern int mm_buf_input;
//extern int n_mm_bufs;

// from rtinput.cpp (now in RTcmix.cpp)
extern int last_input_index;

void RTcmix::set_wmmcontext(int objno)
{
	theContext[objno].NCHANS = NCHANS;
	theContext[objno].RTBUFSAMPS = bufsamps();
	theContext[objno].audioNCHANS = audioNCHANS;
	theContext[objno].SR = sr();
	theContext[objno].runToOffset = runToOffset;
	// BGGx ww -- bufOffset isn't used any more?
	//theContext[objno].bufOffset = bufOffset;
// BGGx
	theContext[objno].bufStartSamp = bufStartSamp;
	theContext[objno].rtInteractive = rtInteractive;
	theContext[objno].rtsetparams_called = rtsetparams_called;
	theContext[objno].audioLoopStarted = audioLoopStarted;
	theContext[objno].audio_config = audio_config;
	theContext[objno].elapsed = elapsed;
	theContext[objno].run_status = run_status;
	theContext[objno].audioDevice = audioDevice;
	theContext[objno].rtHeap = rtHeap;
	theContext[objno].rtQueue = rtQueue;
	theContext[objno].rt_list = rt_list;
	theContext[objno].output_data_format = output_data_format;
	theContext[objno].output_header_type = output_header_type;
	theContext[objno].normalize_output_floats = normalize_output_floats;
	theContext[objno].is_float_format = is_float_format;
	theContext[objno].rtoutsfname = rtoutsfname;
	theContext[objno].audioin_buffer = audioin_buffer;
	theContext[objno].aux_buffer = aux_buffer;
	theContext[objno].out_buffer = out_buffer;
	theContext[objno].rtrecord = rtrecord;
	theContext[objno].rtfileit = rtfileit;
	theContext[objno].rtoutfile = rtoutfile;
	theContext[objno].inputFileTable = inputFileTable;
	theContext[objno].max_input_fds = max_input_fds;
	theContext[objno].last_input_index = last_input_index;
	theContext[objno].AuxToAuxPlayList = AuxToAuxPlayList;
	theContext[objno].ToOutPlayList = ToOutPlayList;
	theContext[objno].ToAuxPlayList = ToAuxPlayList;
	theContext[objno].Inst_Bus_Config = Inst_Bus_Config;
	theContext[objno].Bus_Config_Status = Bus_Config_Status;
	theContext[objno].busCount = busCount;
	theContext[objno].BusConfigs = BusConfigs;

// BGGx
/*
	theContext[objno].Bus_In_Config = Bus_In_Config;
	theContext[objno].HasChild = HasChild;
	theContext[objno].HasParent = HasParent;
	theContext[objno].AuxInUse = AuxInUse;
	theContext[objno].AuxOutInUse = AuxOutInUse;
	theContext[objno].OutInUse = OutInUse;
	theContext[objno].RevPlay = RevPlay;
*/
	theContext[objno]._functionRegistry = _functionRegistry;
	theContext[objno]._func_list = _func_list;


// from gen/makegen.c
/*
	theContext[objno].ngens = ngens;
	theContext[objno].farrays = farrays;
	theContext[objno].sizeof_farray = sizeof_farray;
	theContext[objno].f_goto = f_goto;
*/
// from main.cpp (pf-field stuff
//	theContext[objno].inletvals = inletvals;

// from minc_functions.c
//	theContext[objno].minc_array = minc_array;
//	theContext[objno].minc_array_size = minc_array_size;
// from minc_functions.c
	theContext[objno].resetval = resetval;
// from main.cpp
	theContext[objno].bang_ready = bang_ready;
// from main.cpp
	theContext[objno].vals_ready = vals_ready;
	theContext[objno].maxmsp_vals = maxmsp_vals;
// from main.cpp -- for printing multiple lines in unity
	theContext[objno].firstprint = firstprint;
	theContext[objno].pcheckptr = pcheckptr;
// BGGx -- copy the internal _print from RTcmix into our context var here
//	theContext[objno].wmmprint = Option::print();
// from main.cpp
// BGGx
//	theContext[objno].mm_bufs = mm_bufs;
//	theContext[objno].mm_buf_input = mm_buf_input;
//	theContext[objno].n_mm_bufs = n_mm_bufs;

// context can be written now
	wmmcontext_busy = 0;
}

void RTcmix::get_wmmcontext(int objno)
{
// don't write the context from outside now
	wmmcontext_busy = 1;

	NCHANS = theContext[objno].NCHANS;
	// BGG -- new API for some of these globals
	//RTBUFSAMPS = theContext[objno].RTBUFSAMPS;
	setRTBUFSAMPS(theContext[objno].RTBUFSAMPS);
	audioNCHANS = theContext[objno].audioNCHANS;
	//SR = theContext[objno].SR;
	setSR(theContext[objno].SR);
	runToOffset = theContext[objno].runToOffset;
	// BGGx ww -- bufOffset isn't used any more?
	// bufOffset = theContext[objno].bufOffset;
// BGGx
	bufStartSamp = theContext[objno].bufStartSamp;
	rtInteractive = theContext[objno].rtInteractive;
	rtsetparams_called = theContext[objno].rtsetparams_called;
	audioLoopStarted = theContext[objno].audioLoopStarted;
	audio_config = theContext[objno].audio_config;
	elapsed = theContext[objno].elapsed;
	run_status = theContext[objno].run_status;
	audioDevice = theContext[objno].audioDevice;
	rtHeap = theContext[objno].rtHeap;
	rtQueue = theContext[objno].rtQueue;
	rt_list = theContext[objno].rt_list;
	output_data_format = theContext[objno].output_data_format;
	output_header_type = theContext[objno].output_header_type;
	normalize_output_floats = theContext[objno].normalize_output_floats;
	is_float_format = theContext[objno].is_float_format;
	rtoutsfname = theContext[objno].rtoutsfname;
	audioin_buffer = theContext[objno].audioin_buffer;
	aux_buffer = theContext[objno].aux_buffer;
	out_buffer = theContext[objno].out_buffer;
	rtrecord = theContext[objno].rtrecord;
	rtfileit = theContext[objno].rtfileit;
	rtoutfile = theContext[objno].rtoutfile;
	inputFileTable = theContext[objno].inputFileTable;
	max_input_fds = theContext[objno].max_input_fds;
	last_input_index = theContext[objno].last_input_index;
	AuxToAuxPlayList = theContext[objno].AuxToAuxPlayList;
	ToOutPlayList = theContext[objno].ToOutPlayList;
	ToAuxPlayList = theContext[objno].ToAuxPlayList;
	Inst_Bus_Config = theContext[objno].Inst_Bus_Config;
	Bus_Config_Status = theContext[objno].Bus_Config_Status;
	busCount = theContext[objno].busCount;
	BusConfigs = theContext[objno].BusConfigs;

// BGGx
/*
	Bus_In_Config = theContext[objno].Bus_In_Config;
	HasChild = theContext[objno].HasChild;
	HasParent = theContext[objno].HasParent;
	AuxInUse = theContext[objno].AuxInUse;
	AuxOutInUse = theContext[objno].AuxOutInUse;
	OutInUse = theContext[objno].OutInUse;
	RevPlay = theContext[objno].RevPlay;
*/
	_functionRegistry = theContext[objno]._functionRegistry;
	_func_list = theContext[objno]._func_list;


// from gen/makegen.c
/*
	ngens = theContext[objno].ngens;
	farrays = theContext[objno].farrays;
	sizeof_farray = theContext[objno].sizeof_farray;
	f_goto = theContext[objno].f_goto;
*/
// from main.cpp (pf-field stuff)
//	inletvals = theContext[objno].inletvals;
// from minc_functions.c
//	minc_array = theContext[objno].minc_array;
//	minc_array_size = theContext[objno].minc_array_size;
// from minc_functions.c
	resetval = theContext[objno].resetval;
// from main.cpp
	bang_ready = theContext[objno].bang_ready;
// from main.cpp
	vals_ready = theContext[objno].vals_ready;
	maxmsp_vals = theContext[objno].maxmsp_vals;
// from main.cpp -- for printing multiple lines in unity
	firstprint = theContext[objno].firstprint;
	pcheckptr = theContext[objno].pcheckptr;
// BGGx -- set the internal _print in RTcmix from our context var here
//        Option::print(theContext[objno].wmmprint);
// from main.cpp
// BGGx
//	mm_bufs = theContext[objno].mm_bufs;
//	mm_buf_input = theContext[objno].mm_buf_input;
//	n_mm_bufs = theContext[objno].n_mm_bufs;
}

void RTcmix::clear_wmmcontext(int objno)
{
// don't write the context from outside now
	wmmcontext_busy = 1;

	theContext[objno].NCHANS = 0;
	theContext[objno].RTBUFSAMPS = 0;
	theContext[objno].audioNCHANS = 0;
	theContext[objno].SR = 0.0;
	theContext[objno].runToOffset = false;
	// BGGx ww -- bufOffset isn't used any more?
	//theContext[objno].bufOffset = 0;
	theContext[objno].bufStartSamp = 0;
	theContext[objno].rtInteractive = 0;
	theContext[objno].rtsetparams_called = 0;
	theContext[objno].audioLoopStarted = 0;
	theContext[objno].audio_config = 1;
	theContext[objno].elapsed = 0;
	theContext[objno].run_status = RT_GOOD;
	theContext[objno].audioDevice = NULL;
	theContext[objno].rtHeap = NULL;
	theContext[objno].rtQueue = NULL;
	theContext[objno].rt_list = NULL;
	theContext[objno].output_data_format = -1;
	theContext[objno].output_header_type = -1;
	theContext[objno].normalize_output_floats = 0;
	theContext[objno].is_float_format = 0;
	theContext[objno].rtoutsfname = NULL;
	theContext[objno].audioin_buffer = NULL;
	theContext[objno].aux_buffer = NULL;
	theContext[objno].out_buffer = NULL;
	theContext[objno].rtrecord = false;
	theContext[objno].rtfileit = 0;
	theContext[objno].rtoutfile = 0;
	theContext[objno].inputFileTable = NULL;
	theContext[objno].max_input_fds = 0;
	theContext[objno].last_input_index = 0;
	theContext[objno].AuxToAuxPlayList = NULL;
	theContext[objno].ToOutPlayList = NULL;
	theContext[objno].ToAuxPlayList = NULL;
	theContext[objno].Inst_Bus_Config = NULL;
	theContext[objno].Bus_Config_Status = NO;
	theContext[objno].busCount = DEFAULT_MAXBUS;
	theContext[objno].BusConfigs = NULL;

// BGGx
/*
	theContext[objno].Bus_In_Config = NULL;
	theContext[objno].HasChild = NULL;
	theContext[objno].HasParent = NULL;
	theContext[objno].AuxInUse = NULL;
	theContext[objno].AuxOutInUse = NULL;
	theContext[objno].OutInUse = NULL;
	theContext[objno].RevPlay = NULL;
*/
	theContext[objno]._functionRegistry = NULL;
	theContext[objno]._func_list = NULL;

// from gen/makegen.c
/*
	theContext[objno].ngens = 0;
	theContext[objno].farrays = NULL;
	theContext[objno].sizeof_farray = NULL;
	theContext[objno].f_goto = NULL;
*/
// from main.cpp (pf-field stuff
//	theContext[objno].inletvals = NULL;
// from minc_functions.c
//	theContext[objno].minc_array = NULL;
//	theContext[objno].minc_array_size = NULL;
// from minc_functions.c
	theContext[objno].resetval = 1000;
// from main.cpp
	theContext[objno].bang_ready = 0;
// from main.cpp
	theContext[objno].vals_ready = 0;
	theContext[objno].maxmsp_vals = NULL;
// from main.cpp -- for printing multiple lines in unity
	theContext[objno].firstprint = 0;
	theContext[objno].pcheckptr = NULL;
// BGGx ww -- set our internal print var to default state (2, rterrors only)
//	theContext[objno].wmmprint = 2;
// BGGx -- set the default internal _print in RTcmix from our context var here
        RTOption::print(theContext[objno].wmmprint);
// from main.cpp
//	theContext[objno].mm_bufs = mm_bufs;
//	theContext[objno].mm_buf_input = mm_buf_input;
//	theContext[objno].n_mm_bufs = n_mm_bufs;
}

// BGGxx
extern "C" {
__declspec(dllexport) int check_context() {
   return(wmmcontext_busy);
}

int set_context(int v) {
	wmmcontext_busy = v;
   return(wmmcontext_busy);
}
}
