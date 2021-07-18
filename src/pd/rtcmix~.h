#include "m_pd.h"
#include "g_canvas.h"    /* just for glist_getfont, bother */

#define DYLIBNAME "rtcmixdylib.so"
#define SCOREEXTENSION "sco"
#define TEMPFILENAME "untitled"

// BGG my recompile
#define VERSION "0.93"

#define MAX_INPUTS 12
#define MAX_OUTPUTS 12
#define MAX_SCRIPTS 20 //how many scripts can we store internally
#define MAX_PINLETS 10
#define MAXSCRIPTSIZE 16384

// JWM: since Tk's openpanel & savepanel both use callback(),
// we use a flag to indicate whether we're loading or writing
enum read_write_flags {
								read,
								write,
								none
};

typedef enum { false, true } bool;

// Compatibility fix for 64bit version of garray stuff
// http://pure-data.svn.sourceforge.net/viewvc/pure-data/trunk/scripts/guiplugins/64bit-warning-plugin/README.txt?revision=17094&view=markup
#if (defined PD_MAJOR_VERSION && defined PD_MINOR_VERSION) && (PD_MAJOR_VERSION > 0 || PD_MINOR_VERSION >= 41)
# define arraynumber_t t_word
# define array_getarray garray_getfloatwords
# define array_get(pointer, index) (pointer[index].w_float)
# define array_set(pointer, index, value) ((pointer[index].w_float)=value)
#else
# define arraynumber_t t_float
# define array_getarray garray_getfloatarray
# define array_get(pointer, index) (pointer[index])
# define array_set(pointer, index, value) ((pointer[index])=value)
#endif

/*** RTCMIX FUNCTIONS ---------------------------------------------------------------------------***/
typedef void (*RTcmix_dylibPtr)(void *);
typedef void (*RTcmixBangCallback)(void *inContext);
typedef void (*RTcmixValuesCallback)(float *values, int numValues, void *inContext);
typedef void (*RTcmixPrintCallback)(const char *printBuffer, void *inContext);
typedef int (*RTcmix_initPtr)();
typedef int (*RTcmix_destroyPtr)();
typedef int (*RTcmix_setparamsPtr)(float sr, int nchans, int vecsize, int recording, int bus_count);
/*
   typedef void (*RTcmix_BangCallbackPtr)(void *inContext);
   typedef void (*RTcmix_ValuesCallbackPtr)(float *values, int numValues, void *inContext);
   typedef void (*RTcmix_PrintCallbackPtr)(const char *printBuffer, void *inContext);
 */
typedef void (*RTcmix_setBangCallbackPtr)(RTcmixBangCallback inBangCallback, void *inContext);
typedef void (*RTcmix_setValuesCallbackPtr)(RTcmixValuesCallback inValuesCallback, void *inContext);
typedef void (*RTcmix_setPrintCallbackPtr)(RTcmixPrintCallback inPrintCallback, void *inContext);
typedef int (*RTcmix_resetAudioPtr)(float sr, int nchans, int vecsize, int recording);
typedef enum _RTcmix_AudioFormat {
								AudioFormat_16BitInt = 1, // 16 bit short integer samples
								AudioFormat_24BitInt = 2, // 24 bit (3-byte) packed integer samples
								AudioFormat_32BitInt = 4, // 32 bit (4-byte) integer samples
								AudioFormat_32BitFloat_Normalized = 8, // single-precision float samples, scaled between -1.0 and 1.0
								AudioFormat_32BitFloat = 16 // single-precision float samples, scaled between -32767.0 and 32767.0
} RTcmix_AudioFormat;
typedef int (*RTcmix_setAudioBufferFormatPtr)(RTcmix_AudioFormat format, int nchans);
// Call this to send and receive audio from RTcmix
typedef int (*RTcmix_runAudioPtr)(void *inAudioBuffer, void *outAudioBuffer, int nframes);
typedef int (*RTcmix_parseScorePtr)(char *theBuf, int buflen);
typedef void (*RTcmix_flushScorePtr)();
typedef int (*RTcmix_setInputBufferPtr)(char *bufname, float *bufstart, int nframes, int nchans, int modtime);
typedef int (*RTcmix_getBufferFrameCountPtr)(char *bufname);
typedef int (*RTcmix_getBufferChannelCountPtr)(char *bufname);
typedef void (*RTcmix_setPFieldPtr)(int inlet, float pval);
typedef void (*checkForBangPtr)();
typedef void (*checkForValsPtr)();
typedef void (*checkForPrintPtr)();

/*** PD FUNCTIONS ---------------------------------------------------------------------------***/

static t_class *rtcmix_tilde_class;

typedef struct _rtcmix_tilde
{
								//header
								t_object x_obj;

								//variables specific to this object
								float srate;                                  //sample rate
								t_int vector_size;
								short num_inputs, num_outputs; //number of inputs and outputs
								short num_pinlets; // number of inlets for dynamic PField control
								float *pfield_in; // values received for dynamic PFields
								t_outlet *outpointer;
								//t_inlet **signal_inlets;
								//t_inlet **pfield_inlets;
								//t_outlet **signal_outlets;

								char *tempfolder;
								float *pd_outbuf;
								float *pd_inbuf;

								// RTcmix dylib access pointers
								RTcmix_dylibPtr RTcmix_dylib;
								RTcmix_initPtr RTcmix_init;
								RTcmix_destroyPtr RTcmix_destroy;
								RTcmix_setparamsPtr RTcmix_setparams;
								RTcmix_setBangCallbackPtr RTcmix_setBangCallback;
								RTcmix_setValuesCallbackPtr RTcmix_setValuesCallback;
								RTcmix_setPrintCallbackPtr RTcmix_setPrintCallback;
								RTcmix_resetAudioPtr RTcmix_resetAudio;
								RTcmix_setAudioBufferFormatPtr RTcmix_setAudioBufferFormat;
								RTcmix_runAudioPtr RTcmix_runAudio;
								RTcmix_parseScorePtr RTcmix_parseScore;
								RTcmix_flushScorePtr RTcmix_flushScore;
								RTcmix_setInputBufferPtr RTcmix_setInputBuffer;
								RTcmix_getBufferFrameCountPtr RTcmix_getBufferFrameCount;
								RTcmix_getBufferChannelCountPtr RTcmix_getBufferChannelCount;
								RTcmix_setPFieldPtr RTcmix_setPField;
								checkForBangPtr checkForBang;
								checkForValsPtr checkForVals;
								checkForPrintPtr checkForPrint;

								// for the rtmix_var() and rtcmix_tilde_varlist() $n variable scheme
#define NVARS 9
								char **var_array;

								// stuff for check_vals
#define MAXDISPARGS 1024 // from rtcmix_tilde H/maxdispargs.h
								float thevals[MAXDISPARGS];
								t_atom valslist[MAXDISPARGS];

								// editor stuff
								char **rtcmix_script;
								t_int current_script;
								bool vars_present;
								char **script_path;
								// since both openpanel and savepanel use the same callback method, we
								// have to differentiate whether the callback refers to an open or a save
								enum read_write_flags rw_flag;
								bool buffer_changed;

								// JWM : canvas objects for callback addressing (needed for openpanel and savepanel)
								t_canvas *x_canvas;
								t_symbol *canvas_path;
								t_symbol *x_s;
								char *editorpath;
								char *libfolder;
								char *dylib;

								// for flushing all events on the queue/heap (resets to new ones inside rtcmix_tilde)
								bool flushflag;
								t_float f;

} t_rtcmix_tilde;


/*** FUNCTION PROTOTYPES ---------------------------------------------------------------------------***/

//setup funcs; this probably won't change, unless you decide to change the number of
//args that the user can input, in which case rtcmix_tilde_new will have to change
void rtcmix_tilde_setup(void);
void *rtcmix_tilde_new(t_symbol *s, int argc, t_atom *argv);
void rtcmix_tilde_dsp(t_rtcmix_tilde *x, t_signal **sp); //, short *count);
t_int *rtcmix_tilde_perform(t_int *w);
void rtcmix_tilde_free(t_rtcmix_tilde *x);

// JWM: for getting bang or float at left inlet only
void rtcmix_tilde_bang(t_rtcmix_tilde *x);
void rtcmix_tilde_float(t_rtcmix_tilde *x, t_float scriptnum);
// JWM: float inlets are rewritten (in a horrible embarassing way) below

//for custom messages
void rtcmix_reference(t_rtcmix_tilde *x);
void rtcmix_info(t_rtcmix_tilde *x);
void rtcmix_flush(t_rtcmix_tilde *x);
void rtcmix_reset(t_rtcmix_tilde *x);
void rtcmix_var(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv);
void rtcmix_varlist(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv);
void rtcmix_editor(t_rtcmix_tilde *x, t_symbol *s);

//for the text editor
void rtcmix_openeditor(t_rtcmix_tilde *x);
void rtcmix_open(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv);
void rtcmix_save(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv);
void rtcmix_read(t_rtcmix_tilde *x, char* fn);
void rtcmix_write(t_rtcmix_tilde *x, char* fn);
void rtcmix_callback(t_rtcmix_tilde *x, t_symbol *s);
void rtcmix_bangcallback(void *inContext);
void rtcmix_valuescallback(float *values, int numValues, void *inContext);
void rtcmix_printcallback(const char *printBuffer, void *inContext);
void rtcmix_setscript(t_rtcmix_tilde *x, t_float s);
void rtcmix_bufset(t_rtcmix_tilde *x, t_symbol *s);
void null_the_pointers(t_rtcmix_tilde *x);
void dlopen_and_errorcheck (t_rtcmix_tilde *x);
//char* var_substition (t_rtcmix_tilde *x, const char* script);
void sub_vars_and_parse (t_rtcmix_tilde *x, const char* script);
char* ReadFile(char *filename);
