/* BGG wmm
	So we're going to try to drag around a 'context' for each
	instantiated rtcmix~ object, and we'll reassign the vars
	with each max/msp call
*/

typedef struct {
	int NCHANS;		// RTcmix::NCHANS
	int RTBUFSAMPS;		// RTcmix::RTBUFSAMPS
	int audioNCHANS;	// RTcmix::audioNCHANS
	float SR;			// RTcmix::SR
	bool runToOffset; // RTcmix::runToOffset = false
	FRAMETYPE bufOffset;  // RTcmix::bufOffset = 0
	FRAMETYPE bufStartSamp;	// RTcmix::bufStartSamp = 0
	int rtInteractive;	// RTcmix::rtInteractive = 0
	int rtsetparams_called;	// RTcmix::rtsetparams_called = 0
	int audioLoopStarted; // RTcmix::audioLoopStarted = 0
	int audio_config;	// RTcmix::audio_config = 1
	FRAMETYPE elapsed;		// RTcmix::elapsed = 0
	RTstatus run_status;	// RTcmix::run_status = RT_GOOD
	AudioDevice *audioDevice; // RTcmix::audioDevice = NULL
	heap* rtHeap;		// RTcmix::rtHeap
	RTQueue *rtQueue;	// RTcmix::rtQueue
	rt_item *rt_list;	// RTcmix::rt_list
	int output_data_format; // RTcmix::output_data_format  = -1
	int output_header_type; // RTcmix::output_header_type       = -1;
	int normalize_output_floats; // RTcmix::normalize_output_floats  = 0;
	int is_float_format; // RTcmix::is_float_format       = 0;
	char *rtoutsfname; // RTcmix::rtoutsfname        = NULL;
	BufPtr *audioin_buffer;	// RTcmix::audioin_buffer
	BufPtr *aux_buffer;	// RTcmix::aux_buffer
	BufPtr *out_buffer;	// RTcmix::out_buffer
	bool rtrecord;		// RTcmix::rtrecord = 0
	int rtfileit; // RTcmix::rtfileit  = 0;
	int rtoutfile; // RTcmix::rtoutfile    = 0;
	InputFile *inputFileTable;	// RTcmix::inputFileTable
	long max_input_fds;  // RTcmix::max_input_fds = 0;
	int last_input_index;	// RTcmix::last_input_index = -1;
	short *AuxToAuxPlayList;	// RTcmix::AuxToAuxPlayList
	short *ToOutPlayList;	// RTcmix::ToOutPlayList
	short *ToAuxPlayList;	// RTcmix::ToAuxPlayList
	BusQueue *Inst_Bus_Config;	// RTcmix::Inst_Bus_Config
	Bool Bus_Config_Status;	// RTcmix::Bus_Config_Status
	int busCount; // RTcmix::busCount = DEFAULT_MAXBUS
	BusConfig *BusConfigs; // RTcmix::BusConfigs = NULL

// BGGX
/*
	CheckNode **Bus_In_Config;	// RTcmix::Bus_In_Config
	Bool *HasChild;		// RTcmix::HasChild;
	Bool *HasParent;	// RTcmix::HasParent;
	Bool *AuxInUse;		// RTcmix::AuxInUse;
	Bool *AuxOutInUse;	// RTcmix::AuxOutInUse;
	Bool *OutInUse;		// RTcmix::OutInUse;
	short *RevPlay;		// RTcmix::RevPlay;
*/


	FunctionEntry *_functionRegistry;	// RTcmix::_functionRegistry
	struct _func *_func_list;	// RTcmix::_func_list
// makegen stuff
	int ngens; // from gen/makegen.c (now in RTcmix.cpp)
	double **farrays; // from gen/makegen.c
	int *sizeof_farray; // "
	int *f_goto; // "
// p-field inlets
//	float *inletvals;
// minc_arrays from minc_functions.c
	double **minc_array;
	double *minc_array_size;
// resetval for the reset() call, from minc_functions.c
	int resetval;
// bang_ready for the check_bang() function in main.cpp
	int bang_ready;
// for the check_vals() in main.cpp
	int vals_ready;
	float *maxmsp_vals;

// BGGx -- for printing multiple lines in Unity
	int firstprint;
	char *pcheckptr;

// for buffer_set in main.cpp
// BGGx
//	mm_buf *mm_bufs;
//	int mm_buf_input;
//	int n_mm_bufs;
} wmmcontext;
