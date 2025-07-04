// rtcmix~ v 2.2, Brad Garton (2/2011)
// uses the RTcmix bundled executable lib, now based on RTcmix-5.6 (OS 15.X support)
// see http://music.columbia.edu/cmc/RTcmix for more info
//
// invaluable assistance writing this max/msp object from Dan Trueman, R. Luke Dubois and Joshua Kit Clayton
//
// new:
// 	-- added PField control capability to work with RTcmix v 4.0
//	-- change MAX_INPUTS and MAX_OUTPUTS to 20
//	-- fixed 'orphan editor window' crashing bug
//	-- fixed the NUMVARS bug
//
// new, as of 6/2006:
// 	-- fixed editor window open/audio on&off crashing bug
//	-- increased TOTOBJS to 78
//	-- defer_low()'d bang outputs from MAXBANG instrument
//	-- added multiple script binbuf storage/copy capability
//	-- switched memory mgmt. from t_getbytes() etc. to sysmem_newptr() etc.
//	-- bang will now trigger the currently active script
//	-- [version] message now prints rtcmix~ verson
//	-- [setscript N] message added to set active script
//
// 8/2006
//	-- fixed bug to allow scripts > 1500 or so (binbuf symbol restrictions)
//
// 3/2007
//  -- hopefully UB!
//
// new, 6/2007
//	-- now UB, support for [buffer~] playback
//
// 12/2007
// -- fixed minor bug in the rtcmix_bang() method that was causing "set to script 19" errors
//
// 9/2008
// -- works on 10.5 using a really screwy scheme to write rtcmix*.so files in /tmp
// -- changed that above, now loading using NSmodules with a single dylib.  No limit on # of rtcmix~'s!
//
// 1/2009
// -- updated to RTcmix v 4.0.1.5, including new insts
// -- max5 version
// -- added [flush] message to reset queue/heap dynamically
//
// 1/2011
// -- changed to dlopen() loading instead of the deprecated NSLoad() approach
// -- nope, changed it back to NSLoad().  Even though deprecated, it maintains *unique* symbol space
//		kept the dlopen() code in here for future use (when RTcmix globals are encapsulated)
// -- updated with RTcmix-maxmsp-4.0.1.6, including new PFBUS features and joel instruments
//
// 2/2011
// -- fixed code in rtgetin() for buffer~ (fdindex == USE_MM_BUF has to come before isInputAudioDevice() check)
//
// 12/2012
// -- working on different error-reporting scheme
//
// 10/2013
// -- fixed the info.plist thing; now working with the latest RTcmix
//
// 12/2013
// -- major rewrite to handle unified "embedded" RTcmix API (D.A.S.)
//
// 10/7/2014
// -- added Ryan Carter's fix for large scripts
//
// 01/24/2015
// -- added remainder of Ryan's fixes.  Updated for new RTcmix API.  Now version 2.0!
//
// 02/12/2015
// -- minor bug fixes
//
// 04/01/2015
// -- make sure MSP_INPUTS and MSP_OUTPUTS match those in MSPAudioDevice, and that they can be configured by the compile.
//
// 04/22/2015
// -- allow input and output counts (renamed MSP_INPUTS and MSP_OUTPUTS) to be configured from build macro.
//		added check for active DAC to rtcmix_dortcmix().  Added memory failure check to script methods.
//
// 07/22/2017
// -- removed use of specialized MSPAudioDevice and switched to using EmbeddedAudioDevice.
//
// 5/30/2020
// v. 2.01
// -- now completely 64-bit, with perform64 method, etc.  used max-sdk-7.3.3
//
// (did v. 2.03 to fix crashing on Big Sur; an alloc problem in the RTcmix code)
//  (max_input_fds in RTcmix.cpp)
//
// 9/1/2022
// v. 2.05
// now works under rosetta on M1 machines, OSX Monterey (or ealier)
// -- had to ditch NSModule for loading the rtcmixdylib.so, using the
// old dlopen() scheme instead (where it copies the rtcmixdylib.so with
// a new name in order to get a private name/data space for each [rtcmix~])
// note:  see the note about rm -f "rtcmixdylib*.so" (tagged with BGGx)
//
// 9/21/2024
// v. 2.1
// Now runs in native M1 mode on machines at least up through MacOSX 14 using
// MaxMSP 8.3.1 or earlier.  Updated to support Max SDK 8.2 using "allow deprecated functions".
// At this point it still will not run under MaxMSP 8.6 due to deprecation of MaxAPI.
// Cleaned up void function prototypes.  Removed commented-out NS* loader code for clarity.
//
// 5/2025
// v. 2.2
// Brad started work on Max 9 upgrade using Max 8.2 SDK
// -- Work finished by Doug Scott.  Will now run on all versions up through 9.  All Max framework
//    dependencies are now resolved at load time.  No rtcmix~ linker dependencies on Max.
//

// Hand-defining these here because these will always be true, and are needed to get the right portions of RTcmix_API.h

#define EMBEDDEDAUDIO 1
#define MAXMSP 1

#define VERSION "2.2"
#define RTcmixVERSION "RTcmix-maxmsp-5.6.0"

#if MAX_SDK_MAJOR_VERSION==6
#include <ext.h>
#include <edit.h>
#else
#include <ext_common.h>
#include <ext_prefix.h>
#include <ext_mess.h>
#include <ext_hashtab.h>
#include <ext_dictionary.h>
#include <ext_maxtypes.h>
#include <ext_sysmem.h>
#include <ext_proto.h>
#include <ext_sysfile.h>
#include <ext_path.h>
#endif
// We force this macro when compiling for Max sdk 8.2 and later.
#ifdef MAX_USE_NEW_HEADERS
#include <ext_post.h>
#endif

#include <ext_obex.h>
#include <z_dsp.h>
#include "string.h"
#include <ext_strings.h>
#include <ext_wind.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include "buffer.h"
#include "../../include/RTcmix_API.h"

// BGG using the dlopen stuff for rosetta
#include <dlfcn.h>
int dylibincr;

// MAXCHANS can be configured in the build to allow for up to 128 audio channels.
// If it is defined, use it to configure the input and output count.  Else, default.

#ifndef MAXCHANS
#define MSP_INPUTS 128
#define MSP_OUTPUTS 32
#else
#if MAXCHANS < 32
#define MSP_INPUTS (MAXCHANS+120)
#else
#define MSP_INPUTS (MAXCHANS+64)
#endif
#define MSP_OUTPUTS MAXCHANS
#endif

#define MAX_SCRIPTS 32	//how many scripts can we store internally

// Brad, here are some things I have turned off and on to get this to work efficiently.
// Whatever I left on was my best solution.  Only one can be #defined at a time!  -DS

// BGG
//#undef RELOAD_DYLIB
//#undef DESTROY_ON_DSP_MESSAGE
//#define RESET_AUDIO_ON_DSP_MESSAGE
//#undef RELOAD_DYLIB
// BGG -- these will work and reset the entire RTcmix engine
#undef RELOAD_DYLIB
#define DESTROY_ON_DSP_MESSAGE
#undef RESET_AUDIO_ON_DSP_MESSAGE

/******* RTcmix stuff *******/
typedef int (*rtcmixinitFunctionPtr)(void);
typedef int (*rtcmixdestroyFunctionPtr)(void);
typedef int (*rtsetparamsFunctionPtr)(float sr, int nchans, int vecsize, int recording, int bus_count);
typedef void (*rtsetprintlevelFunctionPtr)(int level);
typedef int (*rtcmixsetaudiobufferformatFunctionPtr)(RTcmix_AudioFormat format, int nchans);

typedef int (*rtcmixrunAudioFunctionPtr)(void *inAudioBuffer, void *outAudioBuffer, int nframes);
typedef int (*rtresetaudioFunctionPtr)(float sr, int nchans, int vecsize, int recording);
typedef int (*parse_scoreFunctionPtr)(char *theBuf, int buflen);
typedef double (*parse_dispatchFunctionPtr)(char *cmd, double *p, int n_args, void *retval);
typedef void (*pfield_setFunctionPtr)(int inlet, float pval);
typedef void (*buffer_setFunctionPtr)(char *bufname, float *bufstart, int nframes, int nchans, int modtime);
typedef int (*flushPtr)(void);
typedef void (*loadinstFunctionPtr)(char *dsolib);
typedef void (*unloadinstFunctionPtr)(void);
typedef void (*setBangCallbackPtr)(RTcmixBangCallback inBangCallback, void *inContext);
typedef void (*setValuesCallbackPtr)(RTcmixValuesCallback inValuesCallback, void *inContext);
typedef void (*setPrintCallbackPtr)(RTcmixPrintCallback inPrintCallback, void *inContext);


static t_class *rtcmix_class = NULL;

typedef struct _rtcmix
{
	//header
	t_pxobject ob;
	
	//variables specific to this object
	float srate;  					// sample rate
	long num_inputs, num_outputs; 	// number of inputs and outputs
	long num_pinlets;				// number of inlets for dynamic PField control
	float in[MSP_INPUTS];			// values of input variables (for float/int inputs, init to 0.0)
	short in_connected[MSP_INPUTS]; //booleans: true if signals connected to the input in question
	//we use this "connected" boolean so that users can connect *either* signals or floats
	//to the various inputs; sometimes it's easier just to have floats, but other times
	//it's essential to have signals.... but we have to know.
	void *outpointer;
	
	/******* RTcmix stuff *******/
	rtcmixinitFunctionPtr rtcmixinit;
	rtcmixdestroyFunctionPtr rtcmixdestroy;
	rtsetparamsFunctionPtr rtsetparams;
	rtsetprintlevelFunctionPtr rtsetprintlevel;
	rtcmixsetaudiobufferformatFunctionPtr rtsetaudiobufferformat;
	rtcmixrunAudioFunctionPtr rtrunaudio;
	rtresetaudioFunctionPtr rtresetaudio;
	parse_scoreFunctionPtr parse_score;
	parse_dispatchFunctionPtr parse_dispatch;
	pfield_setFunctionPtr pfield_set;
	buffer_setFunctionPtr buffer_set;
	flushPtr flush;
	loadinstFunctionPtr loadinst;
	unloadinstFunctionPtr unloadinst;
	setBangCallbackPtr setbangcallback;
	setValuesCallbackPtr setvaluescallback;
	setPrintCallbackPtr setprintcallback;
	
	
	// for load of the rtcmixdylib.so
	// now use dlopen()
	// for the load of rtcmixdylibN.so
	 int dylibincr; // the instance copy of dylibincr for ths dlopen()
	 void *rtcmixdylib;
	 // for the full path to the rtcmixdylib.so file
#define PATHNAME_LEN 1024
	 char pathname[PATHNAME_LEN]; // probably should be malloc'd
	
	// space for these malloc'd in rtcmix_dsp()
	float *maxmsp_outbuf;
	float *maxmsp_inbuf;
	
	// script buffer pointer for large binbuf restores
	char *restore_buf_ptr;
	
	// for the rtmix_var() and rtcmix_varlist() $n variable scheme
#define NVARS 9
	float var_array[NVARS];
	Boolean var_set[NVARS];
	
	// stuff for check_vals
#define MAXDISPARGS 1024 // from RTcmix H/maxdispargs.h
	float thevals[MAXDISPARGS];
	t_atom valslist[MAXDISPARGS];
	
	// editor stuff
	t_object m_obj;
	t_object *m_editor;
	char *rtcmix_script[MAX_SCRIPTS], s_name[MAX_SCRIPTS][256];
	long rtcmix_script_len[MAX_SCRIPTS];
	short current_script, path[MAX_SCRIPTS];
	
	// for flushing all events on the queue/heap (resets to new ones inside RTcmix)
	int flushflag;
	
	// for debugging mode
	int debugflag;

	int audioConfigured;	// if 1, we have already configured audio at least once
	
	// for setting "loadinst" mode, allowing dynamic loading of RTcmix instrumens.
    // BGGx -- doesn't really work at present
	// NOTE:  Only 1 rtcmix~ can be instantiated with this!
	int loadinstflag;
} t_rtcmix;

// for where the rtcmix-dylibs folder is located
char *mpathptr;
char mpathname[1024]; // probably should be malloc'd


/****PROTOTYPES****/

//setup funcs; this probably won't change, unless you decide to change the number of
//args that the user can input, in which case rtcmix_new will have to change
void *rtcmix_new(long num_inoutputs, long num_additional);
void rtcmix_dsp64(t_rtcmix *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
int rtcmix_load_dylib(t_rtcmix *x); // loads the main rtcmixdylib.so lib
void rtcmix_perform64(t_rtcmix *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void rtcmix_assist(t_rtcmix *x, void *b, long m, long a, char *s);
void rtcmix_free(t_rtcmix *x);

//for getting floats, ints or bangs at inputs
void rtcmix_float(t_rtcmix *x, double f);
void rtcmix_int(t_rtcmix *x, long i);
void rtcmix_bang(t_rtcmix *x);
void rtcmix_dobangout(t_rtcmix *x, Symbol *s, short argc, t_atom *argv); // for the defer

void rtcmix_bangcallback(void *inContext);
void rtcmix_valuescallback(float *values, int numValues, void *inContext);
void rtcmix_printcallback(const char *printBuffer, void *inContext);

//for custom messages + support functions
void rtcmix_version(t_rtcmix *x);
void rtcmix_text(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_dotext(t_rtcmix *x, Symbol *s, short argc, Atom *argv); // for the defer
void rtcmix_badquotes(char *cmd, char *buf); // this is to check for 'split' quoted params, called in rtcmix_dotext
void rtcmix_rtcmix(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_dortcmix(t_rtcmix *x, Symbol *s, short argc, Atom *argv); // for the defer
void rtcmix_var(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_varlist(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_bufset(t_rtcmix *x, t_symbol *s);
void rtcmix_flush(t_rtcmix *x);
void rtcmix_loadinst(t_rtcmix *x, t_symbol *s);
void rtcmix_loadset(t_rtcmix *x, long fl);
void rtcmix_debug(t_rtcmix *x);
void rtcmix_dprint(t_rtcmix *x, const char *format, ...);

//for the text editor
void rtcmix_edclose (t_rtcmix *x, char **text, long size);
void rtcmix_dblclick(t_rtcmix *x);
void rtcmix_goscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_dogoscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv); // for the defer
void rtcmix_openscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_setscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_read(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_doread(t_rtcmix *x, Symbol *s, short argc, t_atom *argv);
void rtcmix_write(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_writeas(t_rtcmix *x, Symbol *s, short argc, Atom *argv);
void rtcmix_dowrite(t_rtcmix *x, Symbol *s, short argc, t_atom *argv);
void rtcmix_okclose (t_rtcmix *x, char *prompt, short *result);

//for binbuf storage of scripts
void rtcmix_save(t_rtcmix *x, void *w);
void rtcmix_restore(t_rtcmix *x, Symbol *s, short argc, Atom *argv);

#ifdef DEBUG_MEMORY_OVERRUNS

#define MEMSTART_COOKIE 0xabcdabcd
#define MEMEND_COOKIE 0xefefefef
struct memcheck { unsigned cookie; unsigned size; };

// Debug memory looks like this:
// MEMSTART SIZE   MEMORYFORUSER MEMEND
// 4bytes  4bytes 'size' bytes  4 bytes

#include <assert.h>

void *newmem(unsigned size)
{
    unsigned long newsize = size + sizeof(struct memcheck) + 4;
    char *cptr = (char *)sysmem_newptr(newsize);
    struct memcheck *m = (struct memcheck *) cptr;
    m->cookie = MEMSTART_COOKIE;                        // tag first 4 bytes
    m->size = newsize;                                 // store size in next 4
    *((unsigned*)(cptr+newsize-4)) = MEMEND_COOKIE;  // tag last 4 bytes (past user end)
    return (void *)(cptr+sizeof(struct memcheck));   // return  offset into memory
}

void freemem(void *ptr)
{
    char *cptr = ((char *)ptr) - sizeof(struct memcheck);     // back up
    struct memcheck *m = (struct memcheck *) cptr;            // cast to our struct
    assert(m->cookie == MEMSTART_COOKIE);                       // check for corruption
    assert(m->size < 10000);                                    // make sure its not some large negative, etc.
    cptr += (m->size - 4);                                      // offset to the last 4 bytes
    assert(*((unsigned*) cptr) == MEMEND_COOKIE);               // check for corruption
    sysmem_freeptr(((char *)ptr) - sizeof(struct memcheck));    // free entire original block
}

#else // DEBUG_MEMORY_OVERRUNS end

void *newmem(unsigned size) { return sysmem_newptr(size); }
void freemem(void *ptr) { sysmem_freeptr(ptr); }

#endif

t_symbol *ps_buffer; // for [buffer~]


/****FUNCTIONS****/

//primary MSP funcs
void ext_main(void *r)
{
	int i;
	short path, rval;

	// BGG used for dlopen(), cp-ing the dylib to a unique name
	dylibincr = 0;

	t_class *c = class_new("rtcmix~", (method)rtcmix_new, (method)rtcmix_free, (long)sizeof(t_rtcmix), 0L, A_DEFLONG, A_DEFLONG, 0);
	
	//standard messages; don't change these
	class_addmethod(c, (method)rtcmix_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)rtcmix_assist,"assist", A_CANT,0);
	
	//our own messages
	class_addmethod(c, (method)rtcmix_version, "version", 0);
	class_addmethod(c, (method)rtcmix_text, "text", A_GIMME, 0);
	class_addmethod(c, (method)rtcmix_rtcmix, "rtcmix", A_GIMME, 0);
	class_addmethod(c, (method)rtcmix_var, "var", A_GIMME, 0);
	class_addmethod(c, (method)rtcmix_varlist, "varlist", A_GIMME, 0);
	class_addmethod(c, (method)rtcmix_bufset, "bufset", A_SYM, 0);
	class_addmethod(c, (method)rtcmix_flush, "flush", 0);
	class_addmethod(c, (method)rtcmix_loadinst, "loadinst", A_SYM, 0);
	class_addmethod(c, (method)rtcmix_loadset, "loadset", A_LONG, 0);
	// "debug" toggles debugging mode
	class_addmethod(c, (method)rtcmix_debug, "debug", 0);
	
	//so we know what to do with floats that we receive at the inputs
    class_addmethod(c, (method)rtcmix_float, "float", A_FLOAT, 0);
	
	// this for ints...
    class_addmethod(c, (method)rtcmix_int, "int", A_LONG, 0);
	
	// trigger scripts
    class_addmethod(c, (method)rtcmix_bang, "bang", 0);
	
	//for the text editor and scripts
    class_addmethod(c, (method)rtcmix_edclose, "edclose", A_CANT, 0);
    class_addmethod(c, (method)rtcmix_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)rtcmix_goscript, "goscript", A_GIMME, 0);
    class_addmethod(c, (method)rtcmix_openscript, "openscript", A_GIMME, 0);
    class_addmethod(c, (method)rtcmix_setscript, "setscript", A_GIMME, 0);
    class_addmethod(c, (method)rtcmix_read, "read", A_GIMME, 0);
    class_addmethod(c, (method)rtcmix_okclose, "okclose", A_CANT, 0);
    class_addmethod(c, (method)rtcmix_write, "savescript", A_GIMME, 0);
	class_addmethod(c, (method)rtcmix_writeas, "savescriptas", A_GIMME, 0);
                    
	// binbuf storage
    class_addmethod(c, (method)rtcmix_save, "save", A_CANT, 0);
    class_addmethod(c, (method)rtcmix_restore, "restore", A_GIMME, 0);

	//gotta have this one....
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	rtcmix_class = c;

	// find the rtcmix-dylibs folder location
	nameinpath("rtcmix-dylibs", &path);
	rval = path_topathname(path, "", mpathname);
	if (rval != 0) {
		error("couldn't find the rtcmix-dylibs folder!");
	}
	else { // this is to find the beginning "/" for root
		if (strncasecmp("Users:", mpathname, 6) == 0) {
			// Exactly replace "Users:" with "/Users"
			mpathname[0] = '/';
			mpathname[1] = 'U';
			mpathname[2] = 's';
			mpathname[3] = 'e';
			mpathname[4] = 'r';
			mpathname[5] = 's';
			mpathptr = mpathname;
		}
		else {
			for (i = 0; i < 1000; i++)
				if (mpathname[i] == '/') break;
			mpathptr = mpathname+i;
		}
		//post("converted to '%s'", mpathptr);
	}

	// BGG -- clean up any leftover rtcmixdylib.so files from a previous crash
	char rm_command[1024]; // should probably be malloc'd
	// BGGx -- for some reason "rm -f rtcmixdylib*.so" doesn't work
	// so I'm doing 100 'by hand'.  Hopefully no-one will exceed this
	// it won't be terrible if they do, just some orphan rtcmixdylibs
	// hanging around.
	// when I do the uRTcmix load thing instead this will go away
	for (int ii = 0; ii < 100; ii++)  {
		sprintf(rm_command, "/bin/rm -f \"%s/rtcmixdylib%d.so\" ", mpathptr, ii);
		system(rm_command);
	}
	
	ps_buffer = gensym("buffer~"); // for [buffer~]
    
	// ho ho!
	post("rtcmix~ -- RTcmix music language, v. %s (%s)", VERSION, RTcmixVERSION);
}


//this gets called when the object is created; everytime the user types in new args, this will get called
void *rtcmix_new(long num_inoutputs, long num_additional)
{
	int i;
	t_rtcmix *x;
		
	// creates the object
	x = (t_rtcmix *)object_alloc(rtcmix_class);

	//constrain number of inputs and outputs
	if (num_inoutputs < 1) num_inoutputs = 1; // no args, use default of 1 channel in/out
	if ((num_inoutputs + num_additional) > MSP_INPUTS) {
		error("sorry, only %d total inlets are allowed!", MSP_INPUTS);
		return(NULL);
	}
	
	x->num_inputs = num_inoutputs;
	x->num_outputs = num_inoutputs;
	x->num_pinlets = num_additional;
	
	if (x) {
		dsp_setup((t_pxobject *)x, num_inoutputs + num_additional);   // MSP inlets: arg is # of inlets and is REQUIRED!
	}
	else {
		error("rtcmix~: Object creation failed!");
		return(NULL);
	}
	
	// binbuf storage
	// this sends the 'restore' message (rtcmix_restore() below)
	gensym("#X")->s_thing = (struct object*)x;

	
	// outputs, right-to-left
	x->outpointer = outlet_new((t_object *)x, 0); // for bangs (from MAXBANG), values + value lists (from MAXMESSAGE)
	
	for (i = 0; i < x->num_outputs; i++) {
		outlet_new(x, "signal");
	}
	
	// initialize some variables; important to do this!
	for (i = 0; i < (x->num_inputs + x->num_pinlets); i++) {
		x->in[i] = 0.0; // this is just arbitrary to initialize
	}

	// This calls RTcmix_init() internally -- should we make this explicit?
	if (rtcmix_load_dylib(x) == -1) {
		error("RTcmix dylib not loaded properly -- this won't work!");
		return(NULL);
	}
	
#ifndef RELOAD_DYLIB
	// If we are not reloading the dylib over and over, we don't have to redo this work each time either!
	// Set up callbacks for bang, values, and printing
	if (x->setbangcallback) {
		x->setbangcallback(rtcmix_bangcallback, x);
	}
	if (x->setvaluescallback) {
		x->setvaluescallback(rtcmix_valuescallback, x);
	}
	if (x->setprintcallback) {
		x->setprintcallback(rtcmix_printcallback, x);
	}
#endif
	
	// set up for the variable-substitution scheme
	for(i = 0; i < NVARS; i++) {
		x->var_set[i] = false;
		x->var_array[i] = 0.0;
	}
	
	// the text editor
	x->m_editor = NULL;
	x->current_script = 0;
	for (i = 0; i < MAX_SCRIPTS; i++) {
		x->s_name[i][0] = 0;
	}
	
	x->flushflag = 0; // [flush] sets flag for call to x->flush() in rtcmix_perform() (after pulltraverse completes)
#if DEBUG_LOGGING
	x->debugflag = 1;
#else
	x->debugflag = 0; // debugging is off by default
#endif
	x->audioConfigured = 0;	// indicates first time, prior to configuring audio
	x->loadinstflag = 0; // set for normal operation (no RTcmix instrument dynloading)
    
    x->ob.z_misc |= Z_NO_INPLACE; // ARG!!!!!!!  Cycling!!!!!!!

	return(x);
}

//this gets called everytime audio is started; even when audio is running, if the user
//changes anything (like deletes a patch cord), audio will be turned off and
//then on again, calling this func.
//Also tells us where the audio vectors are and how big they are
void rtcmix_dsp64(t_rtcmix *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	rtcmix_dprint(x, "rtcmix_dsp64() called");
    
	int i;

	object_method(dsp64, gensym("dsp_add64"), x, rtcmix_perform64, 0, NULL);
	
	// set sample rate
	x->srate = samplerate;
	
	int connectedInputs = 0;
	
	// check to see if there are signals connected to the various inputs
    for(i = 0; i < x->num_inputs; i++) { // check our signal inputs only
		x->in_connected[i] = count[i];
		connectedInputs += count[i];
	}
	rtcmix_dprint(x, "rtcmix_dsp: %d connected inputs", connectedInputs);
	
#if defined(RESET_AUDIO_ON_DSP_MESSAGE) // uses old perform methodology!
	if (x->audioConfigured) {
		// This function destroys and rebuilds the AudioDevice and the audio buffers.
		if (x->rtresetaudio) {
			rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_resetAudio()");
			x->rtresetaudio(x->srate, x->num_outputs, sp[0]->s_n, 1);
		}
	}
	else {
		if (x->rtsetaudiobufferformat)
		{
			rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_setAudioBufferFormat()");
			x->rtsetaudiobufferformat(AudioFormat_32BitFloat_Normalized, x->num_outputs);
		}
		if (x->rtsetparams)
		{
			rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_setParams()");
			x->rtsetparams(x->srate, x->num_outputs, sp[0]->s_n, 1, 0);
			x->audioConfigured = 1;
		}
	}
#endif
	
#ifdef RELOAD_DYLIB
	// load the dylib
	rtcmix_load_dylib(x);
#elif defined(DESTROY_ON_DSP_MESSAGE)
	if (x->rtcmixdestroy) {
		rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_destroy()");
		x->rtcmixdestroy();
	}
	if (x->rtcmixinit) {
		rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_init()");
		x->rtcmixinit();
	}
#endif

	x->maxmsp_inbuf = malloc(sizeof(float) *  maxvectorsize * x->num_inputs);
	x->maxmsp_outbuf = malloc(sizeof(float) * maxvectorsize * x->num_outputs);
	
	// zero out these buffers to be safe
	for (i = 0; i < (maxvectorsize * x->num_inputs); i++) x->maxmsp_inbuf[i] = 0.0;
	for (i = 0; i < (maxvectorsize * x->num_outputs); i++) x->maxmsp_outbuf[i] = 0.0;

#if defined(RELOAD_DYLIB) || defined(DESTROY_ON_DSP_MESSAGE)
	// This is done if we are either reloading the dylib, or destroying everything each time.
	if (x->rtsetaudiobufferformat)
	{
		rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_setAudioBufferFormat with %d output channels", (int)x->num_outputs);
		x->rtsetaudiobufferformat(AudioFormat_32BitFloat_Normalized, x->num_outputs);
	}
	if (x->rtsetparams)
	{
		rtcmix_dprint(x, "rtcmix_dsp calling RTcmix_setparams()");
		x->rtsetparams(x->srate, x->num_outputs, maxvectorsize, 1, 0);
	}
#endif
	
#ifdef RELOAD_DYLIB
	// This stuff only needs to be done here if we are actually overwriting the dylib each time.
	// Set up callbacks for bang, values, and printing
	if (x->setbangcallback) {
		x->setbangcallback(rtcmix_bangcallback, x);
	}
	if (x->setvaluescallback) {
		x->setvaluescallback(rtcmix_valuescallback, x);
	}
	if (x->setprintcallback) {
		x->setprintcallback(rtcmix_printcallback, x);
	}
#endif
}


// load the main rtcmixdylib.so, returns 0 if loaded, -1 if not
int rtcmix_load_dylib(t_rtcmix *x)
{
	rtcmix_dprint(x, "rtcmix_load_dylib() called");
	// these are the entry function pointers in to the rtcmixdylib.so lib
	x->rtcmixinit = NULL;
	x->rtcmixdestroy = NULL;
	x->rtsetparams = NULL;
	x->rtsetprintlevel = NULL;
	x->rtsetaudiobufferformat = NULL;
	x->rtrunaudio = NULL;
	x->rtresetaudio = NULL;
	x->parse_score = NULL;
	x->parse_dispatch = NULL;
	x->pfield_set = NULL;
	x->buffer_set = NULL;
	x->flush = NULL;
	x->loadinst = NULL;
	x->unloadinst = NULL;
	x->setbangcallback = NULL;
	x->setvaluescallback = NULL;
	x->setprintcallback = NULL;
	
	// RTcmix stuff: dlsym code

	 char cp_command[PATHNAME_LEN]; // should probably be malloc'd
	 
	 // first time around:
	 x->dylibincr = dylibincr++; // keep track of rtcmixdylibN.so for copy/load
	 
	 // full path to the rtcmixdylib.so file
	 snprintf(x->pathname, PATHNAME_LEN, "%s/rtcmixdylib%d.so", mpathptr, x->dylibincr);

	 // ok, this is fairly insane.  To guarantee a fully-isolated namespace with dlopen(), we need
	 // a totally *unique* dylib, so we copy this.  Deleted in rtcmix_free() below
	 // RTLD_LOCAL doesn't do it all - probably the global vars in RTcmix.

     const char *rtlib_name = "BASE_rtcmixdylib.so";
     snprintf(cp_command, PATHNAME_LEN, "cp \"%s/%s\" \"%s\"", mpathptr, rtlib_name, x->pathname);
     system(cp_command);
	 
	 // reload, this reinits the RTcmix queue, etc.
	 dlclose(x->rtcmixdylib);
	 
	 // load the dylib
	 x->rtcmixdylib = dlopen(x->pathname,  RTLD_NOW | RTLD_LOCAL);
	 
	 // find the main entry to be sure we're cool...
	 x->rtcmixinit = dlsym(x->rtcmixdylib, "RTcmix_init");
	 if (x->rtcmixinit)
		x->rtcmixinit();
	 else error("rtcmix~ could not call RTcmix_init()");

	x->rtcmixdestroy = dlsym(x->rtcmixdylib, "RTcmix_destroy");
	if (!(x->rtcmixdestroy))
		error("rtcmix~ could not find RTcmix_destroy()");
	 
	 x->rtsetparams = dlsym(x->rtcmixdylib, "RTcmix_setparams");
	 if (!(x->rtsetparams))
		 error("rtcmix~ could not find RTcmix_setparams()");

	 x->rtsetprintlevel = dlsym(x->rtcmixdylib, "RTcmix_setPrintLevel");
	 if (!(x->rtsetprintlevel))
		 error("rtcmix~ could not find RTcmix_setPrintLevel()");

	x->rtsetaudiobufferformat = dlsym(x->rtcmixdylib, "RTcmix_setAudioBufferFormat");
	if (!(x->rtsetaudiobufferformat))
		error("rtcmix~ could not find RTcmix_setAudioBufferFormat()");

	x->rtrunaudio = dlsym(x->rtcmixdylib, "RTcmix_runAudio");
	if (!(x->rtrunaudio))
		error("rtcmix~ could not find RTcmix_runAudio()");

	x->rtresetaudio = dlsym(x->rtcmixdylib,  "RTcmix_resetAudio");
	if (!(x->rtresetaudio))
		error("rtcmix~ could not find RTcmix_resetAudio()");

	x->parse_score = dlsym(x->rtcmixdylib, "RTcmix_parseScore");
	if (!(x->parse_score))
		error("rtcmix~ could not find RTcmix_parseScore");

	x->setbangcallback = dlsym(x->rtcmixdylib, "RTcmix_setBangCallback");
	if (!(x->setbangcallback))
		error("rtcmix~ could not find RTcmix_setBangCallback()");

	x->setvaluescallback = dlsym(x->rtcmixdylib, "RTcmix_setValuesCallback");
	if (!(x->setvaluescallback))
		error("rtcmix~ could not find RTcmix_setValuesCallback()");
 
	x->parse_dispatch = dlsym(x->rtcmixdylib, "parse_dispatch");
	if (!(x->parse_dispatch))
		error("rtcmix~ could not find parse_dispatch()");

	x->setprintcallback = dlsym(x->rtcmixdylib, "RTcmix_setPrintCallback");
	if (!(x->setprintcallback))
		error("rtcmix~ could not find RTcmix_setPrintCallback()");

	x->pfield_set = dlsym(x->rtcmixdylib, "RTcmix_setPField");
	if (!(x->pfield_set))
		error("rtcmix~ could not find RTcmix_setPField()");

	x->buffer_set = dlsym(x->rtcmixdylib, "RTcmix_setInputBuffer");
	if (!(x->buffer_set))
		error("rtcmix~ could not find RTcmix_setInputBuffer()");

	x->flush =  dlsym(x->rtcmixdylib, "RTcmix_flushScore");
	if (!(x->flush))
		error("rtcmix~ could not find RTcmix_flushScore()");

	x->loadinst = dlsym(x->rtcmixdylib, "loadinst");
	if (!(x->loadinst))
		error("rtcmix~ could not find loadinst()");
	
	x->unloadinst = dlsym(x->rtcmixdylib, "unloadinst");
	if (!(x->unloadinst))
		error("rtcmix~ could not find unloadinst()");

	
	rtcmix_dprint(x, "rtcmix_load_dylib() complete");
	
	return(0);
}

//this is where the action is
//we get vectors of samples (n samples per vector), process them and send them out
void rtcmix_perform64(t_rtcmix *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	// NO debugging post here; this one gets called all the time
	
	double *in[MSP_INPUTS]; 		//pointers to the input vectors
	double *out[MSP_OUTPUTS];	//pointers to the output vectors
	
	//random local vars
	int i, j, k;

	//check to see if we have a signal or float message connected to input
	//then assign the pointer accordingly
    // msp ins and outs are not interleaved -- each ins[i] and outs[i] points to a channel of samples
	for (i = 0; i < x->num_inputs; i++) { // don't use numins, it also include pfield inputs (proxies)
        if (x->in_connected[i])
            in[i] = ins[i]; // each input in[] points to a channel
        out[i] = outs[i];
	}
    for (i = 0; i < numouts; i++) {
        out[i] = outs[i];
    }

	k = 0;
	// copy input from MAX into transfer buffer
    for (i = 0; i < sampleframes; i++) {
        for (j = 0; j < x->num_inputs; j++) {
            if (x->in_connected[j]) {
                (x->maxmsp_inbuf)[k++] = *in[j]++;
            }
            else {
                (x->maxmsp_inbuf)[k++] = x->in[j];
            }
        }
    }
    
	// RTcmix stuff
	// this drives the RTcmix sample-computing engine
	x->rtrunaudio(x->maxmsp_inbuf, x->maxmsp_outbuf, sampleframes);
    
    k = 0;
    // copy output from RTcmix transfer buffer
    for (i = 0; i < sampleframes; i++) {
        for(j = 0; j < x->num_outputs; j++) {
            *out[j]++ = (x->maxmsp_outbuf)[k++];
        }
    }
}

// the deferred bang output
void rtcmix_dobangout(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_dobangout() called");
	
	outlet_bang(x->outpointer);
	
	rtcmix_dprint(x, "rtcmix_dobangout() complete");
}


//tells the user about the inputs/outputs when mousing over them
void rtcmix_assist(t_rtcmix *x, void *b, long m, long a, char *s)
{
	
	if (m == 1) {
		if (a == 0) sprintf(s, "signal/text (score commands) in");
		else sprintf(s, "signal/pfieldvals in");
	}
	if (m == 2) {
		if (a < x->num_inputs) sprintf(s, "signal out");
		else sprintf(s, "bang, float or float-list out");
	}
}


// here's my free function
void rtcmix_free(t_rtcmix *x)
{
	rtcmix_dprint(x, "rtcmix_free() called");
	
	// BGG kept dlopen() stuff in in case NSLoad gets dropped
	 char rm_command[1024]; // should probably be malloc'd

	// BGG -- remover the dylib cp'd for this instance
	 dlclose(x->rtcmixdylib);
	 sprintf(rm_command, "/bin/rm -f \"%s\" ", x->pathname);
	 system(rm_command);
	 dlclose(x->rtcmixdylib);
	
	// close any open editor windows
	if (x->m_editor)
		freeobject((t_object *)x->m_editor);
	x->m_editor = NULL;
	
	// Free the RTcmix system
	if (x->rtcmixdestroy) {
		rtcmix_dprint(x, "rtcmix_free calling RTcmix_destroy()");
// BGGx -- something wrong in the main rtcmix code causing frashes
//		x->rtcmixdestroy();
	}
	
	// free our transfer buffers
	free(x->maxmsp_inbuf);
	free(x->maxmsp_outbuf);
	x->maxmsp_inbuf = x->maxmsp_outbuf = NULL;

	dsp_free((t_pxobject *)x);
	
	rtcmix_dprint(x, "rtcmix_free() complete");
}


//this gets called whenever a float is received at *any* input
// used for the PField control inlets
void rtcmix_float(t_rtcmix *x, double fval)
{
    long inlet_number = proxy_getinlet((t_object *)x);
    
    if (inlet_number < x->num_inputs) {
        x->in[inlet_number] = fval; // constant will go in for this signal
    }
    else {
        x->pfield_set((inlet_number - x->num_inputs) + 1, fval);
    }
}


//this gets called whenever an int is received at *any* input
// used for the PField control inlets
void rtcmix_int(t_rtcmix *x, long ival)
{
    long inlet_number = proxy_getinlet((t_object *)x);
    
    if (inlet_number < x->num_inputs) {
        x->in[inlet_number] = (float)ival; // constant will go in for this signal
    }
    else {
        x->pfield_set((inlet_number - x->num_inputs) + 1, (float)ival);
    }
}

void rtcmix_bangcallback(void *inContext)
{
	t_rtcmix *x = (t_rtcmix *) inContext;
	// got a pending bang from MAXBANG()
	defer_low(x, (method)rtcmix_dobangout, (Symbol *)NULL, 0, (Atom *)NULL);
}

void rtcmix_valuescallback(float *values, int numValues, void *inContext)
{
	t_rtcmix *x = (t_rtcmix *) inContext;
	int i;
	// BGG -- I should probably defer this one and the error posts also.  So far not a problem...
	if (numValues == 1)
		outlet_float(x->outpointer, (double)(values[0]));
	else {
		for (i = 0; i < numValues; i++) A_SETFLOAT((x->valslist)+i, values[i]);
		outlet_list(x->outpointer, 0L, numValues, x->valslist);
	}
}

void rtcmix_printcallback(const char *printBuffer, void *inContext)
{
	const char *pbufptr = printBuffer;
	while (strlen(pbufptr) > 0) {
		post("RTcmix: %s", pbufptr);
		pbufptr += (strlen(pbufptr) + 1);
	}
}


// bang triggers the current working script
void rtcmix_bang(t_rtcmix *x)
{
	rtcmix_dprint(x, "rtcmix_bang() called");

	Atom a[1];
	
	if (x->flushflag == 1) return; // heap and queue being reset
	
	a[0].a_w.w_long = x->current_script;
	a[0].a_type = A_LONG;
	defer_low(x, (method)rtcmix_dogoscript, NULL, 1, a);
	
	rtcmix_dprint(x, "rtcmix_bang() complete");
}


// print out the rtcmix~ version
void rtcmix_version(t_rtcmix *x)
{
	post("rtcmix~, v. %s by Brad Garton/Doug Scott (%s)", VERSION, RTcmixVERSION);
}


// see the note for rtcmix_dotext() below
void rtcmix_text(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_text() called");
	
	if (x->flushflag == 1) return; // heap and queue being reset
	defer_low(x, (method)rtcmix_dotext, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_text() complete");
}


// what to do when we get the message "text"
// rtcmix~ scores come from the [textedit] object this way
void rtcmix_dotext(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_dotext() called");
	
	short i, varnum;
	char thebuf[8192]; // should #define these probably
	char xfer[8192];
	char *bptr;
	int nchars;
	int top;
	
	bptr = thebuf;
	nchars = 0;
	top = 0;
	
	for (i=0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				sprintf(xfer, " %ld", (long)argv[i].a_w.w_long);
				break;
			case A_FLOAT:
				sprintf(xfer, " %lf", argv[i].a_w.w_float);
				break;
			case A_DOLLAR:
				varnum = argv[i].a_w.w_long;
				if ( !(x->var_set[varnum-1]) ) error("variable $%d has not been set yet, using 0.0 as default",varnum);
				sprintf(xfer, " %lf", x->var_array[varnum-1]);
				break;
			case A_SYM:
				if (top == 0) { sprintf(xfer, "%s", argv[i].a_w.w_sym->s_name); top = 1;}
				else sprintf(xfer, " %s", argv[i].a_w.w_sym->s_name);
				break;
			case A_SEMI:
				sprintf(xfer, ";");
				break;
			case A_COMMA:
				sprintf(xfer, ",");
		}
		strcpy(bptr, xfer);
		nchars = strlen(xfer);
		bptr += nchars;
	}
	
	// don't send if the dacs aren't turned on, unless it is a system() <------- HACK HACK HACK!
	if ( (sys_getdspstate() == 1) || (strncmp(thebuf, "system", 6) == 0) ) {
		// ok, here's the deal -- when cycling tokenizes the message stream, if a quoted param
		// (like a pathname to a soundfile) contains any spaces it will be split into to separate,
		// unquoted Symbols.  The function below repairs this for certain specific rtcmix commands.
		// Not really elegant, but I don't know of another solution at present
		rtcmix_badquotes("rtinput", thebuf);
		rtcmix_badquotes("system", thebuf);
		rtcmix_badquotes("dataset", thebuf);
		
		if (x->parse_score(thebuf, strlen(thebuf)) != 0) {
			error("problem parsing RTcmix script");
			rtcmix_dprint(x, "script: \"%s\"", thebuf);
		}
	}
	else {
		post("DACs must be on to parse an RTcmix script");
	}
	rtcmix_dprint(x, "rtcmix_dotext() complete");
}


// see the note in rtcmix_dotext() about why I have to do this
void rtcmix_badquotes(char *cmd, char *thebuf)
{
	int i;
	char *rtinputptr;
	Boolean badquotes, checkon;
	int clen;
	char tbuf[8192];
	
	// jeez this just sucks big giant easter eggs
	badquotes = false;
	rtinputptr = strstr(thebuf, cmd); // find (if it exists) the instance of the command that may have a split in the quoted param
	if (rtinputptr) {
		rtinputptr += strlen(cmd);
		clen = strlen(thebuf) - (rtinputptr-thebuf);
		checkon = false;
		for (i = 0; i < clen; i++) { // start from the command and look for spaces, between ( )
			if (*rtinputptr++ == '(' ) checkon = true;
			if (checkon) {
				if ((int)*rtinputptr == 34) { // we found a quote, so its cool and we can stop
					i = clen;
				} else if (*rtinputptr == ')' ) {  // uh oh, no quotes and this command expects them
					badquotes = true;
					i = clen;
				}
			}
		}
	}
	
	// lordy, look at this code.  I wish cycling would come up with an unaltered buf-passing type
	if (badquotes) { // now we're gonna put in the missing quotes
		rtinputptr = strstr(thebuf, cmd);
		rtinputptr += strlen(cmd);
		checkon = false;
		for (i = 0; i < clen; i++) {
			if (*rtinputptr++ == '(' ) checkon = true;
			if (checkon)
				if (*rtinputptr != ' ') i = clen;
		} // at this point we're at the beginning of the should-be-quoted param in the buffer
		
		// so we copy it to a temporary buffer, insert a quote...
		strcpy(tbuf, rtinputptr);
		*rtinputptr++ = 34;
		strcpy(rtinputptr, tbuf);
		
		// and then put in the rest of the param, inserting a quote at the end of the should-be-quoted param
		clen = strlen(thebuf) - (rtinputptr-thebuf);
		for (i = 0; i < clen; i++)
			if (*(++rtinputptr) == ')' ) {
				while (*(--rtinputptr) == ' ') { }
				rtinputptr++;
				i = clen;
			}
		
		// and this splices the modified, happily-quoted-param buffer back to the buf we give to rtcmix
		strcpy(tbuf, rtinputptr);
		*rtinputptr++ = 34;
		strcpy(rtinputptr, tbuf);
	}
}


// see the note for rtcmix_dortcmix() below
void rtcmix_rtcmix(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_rtcmix() called");
	
	if (x->flushflag == 1) return; // heap and queue being reset
	defer_low(x, (method)rtcmix_dortcmix, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_rtcmix() complete");
}


// what to do when we get the message "rtcmix"
// used for single-shot RTcmix scorefile commands
void rtcmix_dortcmix(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_dortcmix() called");
	
	short i;
	double p[1024]; // should #define this probably
	char *cmd = NULL;
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				p[i-1] = (double)(argv[i].a_w.w_long);
				break;
			case A_FLOAT:
				p[i-1] = (double)(argv[i].a_w.w_float);
				break;
			case A_SYM:
				cmd = argv[i].a_w.w_sym->s_name;
				break;
		}
	}
	
	if ( (sys_getdspstate() == 1) || (strncmp(cmd, "system", 6) == 0) ) {
		x->parse_dispatch(cmd, p, argc-1, NULL);
	}
	else {
		post("DACs must be on to send an RTcmix command");
	}
	
	rtcmix_dprint(x, "rtcmix_dortcmix() complete");
}


// the "var" message allows us to set $n variables imbedded in a scorefile with varnum value messages
void rtcmix_var(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_var() called");
	
	short i, varnum;
	
	for (i = 0; i < argc; i += 2) {
		varnum = argv[i].a_w.w_long;
		if ( (varnum < 1) || (varnum > NVARS) ) {
			error("only vars $1 - $9 are allowed");
			return;
		}
		x->var_set[varnum-1] = true;
		switch (argv[i+1].a_type) {
			case A_LONG:
				x->var_array[varnum-1] = (float)(argv[i+1].a_w.w_long);
				break;
			case A_FLOAT:
				x->var_array[varnum-1] = argv[i+1].a_w.w_float;
		}
	}
	
	rtcmix_dprint(x, "rtcmix_var() complete");
}


// the "varlist" message allows us to set $n variables imbedded in a scorefile with a list of positional vars
void rtcmix_varlist(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_varlist() called");
	
	short i;
	
	if (argc > NVARS) {
		error("asking for too many variables, only setting the first 9 ($1-$9)");
		argc = NVARS;
	}
	
	for (i = 0; i < argc; i++) {
		x->var_set[i] = true;
		switch (argv[i].a_type) {
			case A_LONG:
				x->var_array[i] = (float)(argv[i].a_w.w_long);
				break;
			case A_FLOAT:
				x->var_array[i] = argv[i].a_w.w_float;
		}
	}
	
	rtcmix_dprint(x, "rtcmix_varlist() complete");
}

// the "bufset" message allows access to a [buffer~] object.  The only argument is the name of the [buffer~]
void rtcmix_bufset(t_rtcmix *x, t_symbol *s)
{
	rtcmix_dprint(x, "rtcmix_bufset() called");
	
	t_buffer *b;
	
	if ((b = (t_buffer *)(s->s_thing)) && ob_sym(b) == ps_buffer) {
		x->buffer_set(s->s_name, b->b_samples, b->b_frames, b->b_nchans, b->b_modtime);
	} else {
		error("rtcmix~: no buffer~ %s", s->s_name);
	}
	
	rtcmix_dprint(x, "rtcmix_bufset() complete");
}


// the "flush" message, remove all executing instruments/notes from the queue
void rtcmix_flush(t_rtcmix *x)
{
	x->flush();		// call this directly
}

// the "debug" message, toggles debug mode on/off
void rtcmix_debug(t_rtcmix *x)
{
	if (x->debugflag == 0) {
		post("rtcmix~: Debugging mode has been turned on. Send message <debug> to turn off.");
		x->debugflag = 1; // turn on debugging
	}
	else {
		post("rtcmix~: Debugging mode has been turned off. Send message <debug> to turn on.");
		x->debugflag = 0; // turn it back off
	}
}

void rtcmix_dprint(t_rtcmix *x, const char *format, ...)
{
	if (x->debugflag == 1) {
		char     buf[1024];
		va_list  args;
		
		va_start(args, format);
		vsnprintf(buf, 1024, format, args);
		va_end(args);
		
		post("rtcmix~: %s", buf);
	}
}

// the "loadinst" message for dynamically loading instruments
// BGG -- this doesn't work all that well
void rtcmix_loadinst(t_rtcmix *x, t_symbol *s)
{
	rtcmix_dprint(x, "rtcmix_loadinst() called");
	
	if (x->loadinstflag == 1) {
		if (sys_getdspobjdspstate((t_object *)x) != 1) {
			error("audio should be on to load an instrument (instrument not loaded)");
			return;
		}
		x->loadinst(s->s_name);
	}
	else
		error("load mode not set, use [loadset 1] to enable");
	
	rtcmix_dprint(x, "rtcmix_loadinst() complete");
}


// the "loadset" message, turn on/off [0/1] dynload-mode
// BGG -- this doesn't work all that well
void rtcmix_loadset(t_rtcmix *x, long fl)
{
	rtcmix_dprint(x, "rtcmix_loadset() called");
	
	// turn off the dacs o allow unloading of current dylibs
	if (sys_getdspobjdspstate((t_object *)x) == 1) {
		dspmess(gensym("stop"));
	}

	if (fl == 0) {
		x->loadinstflag = 0;
		post("load mode unset, dynloading of RTcmix instruments turned off");
	} else {
		x->loadinstflag = 1;
		post("load mode set, dynloading of RTcmix instruments turned on");
		post("NOTE: only 1 rtcmix~ object should be instantiated in this mode!");
	}
	
	rtcmix_dprint(x, "rtcmix_loadset() complete");
}


// here is the text-editor buffer stuff, go dan trueman go!
// used for rtcmix~ internal buffers
void rtcmix_edclose (t_rtcmix *x, char **text, long size)
{
	rtcmix_dprint(x, "rtcmix_edclose() called with text \"%.16s\"... with size %ld", *text, size);
	
	if (x->rtcmix_script[x->current_script]) {
		freemem((void *)x->rtcmix_script[x->current_script]);
		x->rtcmix_script[x->current_script] = 0;
	}
	x->rtcmix_script_len[x->current_script] = size;
	x->rtcmix_script[x->current_script] = (char *)newmem((size+1) * sizeof(char)); // size+1 so we can add '\0' at end
	if (x->rtcmix_script[x->current_script]) {
		strncpy(x->rtcmix_script[x->current_script], *text, size);
		x->rtcmix_script[x->current_script][size] = '\0'; // add the terminating '\0'
		x->m_editor = NULL;
	}
	else {
		error("rtcmix~:  problem allocating memory for current script (size %ld+1)", size);
	}
	
	rtcmix_dprint(x, "rtcmix_edclose() complete");
}


void rtcmix_okclose (t_rtcmix *x, char *prompt, short *result)
{
	rtcmix_dprint(x, "rtcmix_okclose() called");
	
	*result = 3; //don't put up dialog box
	
	rtcmix_dprint(x, "rtcmix_okclose() complete");
	
	return;
}


// open up an ed window on the current buffer
void rtcmix_dblclick(t_rtcmix *x)
{
	rtcmix_dprint(x, "rtcmix_dblclick() called");
	
	char title[80];
	
	if (x->m_editor) {
		if(x->rtcmix_script[x->current_script])
			object_method(x->m_editor, gensym("settext"), x->rtcmix_script[x->current_script], gensym("utf-8"));
	} else {
		x->m_editor = object_new(CLASS_NOBOX, gensym("jed"), (t_object *)x, 0);
		sprintf(title,"script_%d", x->current_script);
		object_attr_setsym(x->m_editor, gensym("title"), gensym(title));
		if(x->rtcmix_script[x->current_script])
			object_method(x->m_editor, gensym("settext"), x->rtcmix_script[x->current_script], gensym("utf-8"));
	}
	
	object_attr_setchar(x->m_editor, gensym("visible"), 1);
	
	rtcmix_dprint(x, "rtcmix_dblclick() complete");
}


// see the note for rtcmix_goscript() below
void rtcmix_goscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_goscript() called");
	
	if (x->flushflag == 1) return; // heap and queue being reset
	defer_low(x, (method)rtcmix_dogoscript, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_goscript() complete");
}

static int count_chars(const char* string, char ch)
{
	int count = 0;
	for(; *string; count += (*string++ == ch)) ;
	return count;
}

// the [goscript N] message will cause buffer N to be sent to the RTcmix parser
void rtcmix_dogoscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_dogoscript() called");
	
	int i,j,temp = 0; // these were previously declared as short, and that causes infinite recursion if i > 32767
	int tval;
	int buflen;
	char *thebuf = NULL;
	
	if (argc == 0) {
		error("rtcmix~: goscript needs a buffer number [0-31]");
		return;
	}
	
	// argument handling for [goscript X] is permissive enough to ignore any character until a number
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				rtcmix_dprint(x, "goscript %i called", temp);
				break;
			case A_FLOAT:
				temp = (int)argv[i].a_w.w_float;
				rtcmix_dprint(x, "goscript %i called", temp);
				break;
		}
	}
	
	if (temp > MAX_SCRIPTS) {
		error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
		temp = MAX_SCRIPTS-1;
	}
	if (temp < 0) {
		error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
		temp = 0;
	}
	x->current_script = temp;
	
	buflen = x->rtcmix_script_len[x->current_script];
	
	if (buflen == 0) {
		error("rtcmix~: you are triggering a 0-length script!");
		return;
	}
	
	// $ variables add length when expanded
	int substitutions = count_chars(x->rtcmix_script[x->current_script], '$');
	
	// We'll leave 16 chars for each
	int outbuflen = buflen + (substitutions * 16);
	
	// make sure there's room for the \0,
	// plus the substitution of \n for those annoying ^M thingies

	if ((thebuf = newmem(outbuflen+1)) == NULL) {
		error("rtcmix~: problem allocating memory for score");
		return;
	}
	
	for (i = 0, j = 0; i < buflen && j < outbuflen; i++) {
		thebuf[j] = *(x->rtcmix_script[x->current_script]+i);
		if ((int)thebuf[j] == 13) {
			thebuf[j] = '\n'; 	// RTcmix wants newlines, not <cr>'s
		}
		// ok, here's where we substitute the $vars
		if (thebuf[j] == '$') {
			sscanf(x->rtcmix_script[x->current_script]+i+1, "%d", &tval);
			if ( !(x->var_set[tval-1]) ) error("variable $%d has not been set yet, using 0.0 as default", tval);
			snprintf(thebuf+j, outbuflen-j, "%f", x->var_array[tval-1]);
			j = strlen(thebuf)-1;
			i++; // skip over the var number in input
		}
		j++;
	}
	thebuf[j] = '\0';
	
	// don't send if the dacs aren't turned on, unless it is a system() <------- HACK HACK HACK!
	if ( (sys_getdspstate() == 1) || (strncmp(thebuf, "system", 6) == 0) ) {
		if (x->parse_score(thebuf, j) != 0) {
			error("problem parsing RTcmix script");
			rtcmix_dprint(x, "script: \"%s\"", thebuf);
		}
	}
	else {
		post("DACs must be on to parse an RTcmix script");
	}
	
	freemem(thebuf);
	
	rtcmix_dprint(x, "rtcmix_dogoscript() complete with expanded script size: %i", j);
}


// [openscript N] will open a buffer N
void rtcmix_openscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_openscript() called");
	
	int i,temp = 0;
	
	if (argc == 0) {
		error("rtcmix~: openscript needs a buffer number [0-19]");
		return;
	}
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				rtcmix_dprint(x, "openscript %i called", temp);
				break;
			case A_FLOAT:
				temp = (int)argv[i].a_w.w_float;
				rtcmix_dprint(x, "openscript %i called", temp);
				break;
		}
	}
	
	if (temp > MAX_SCRIPTS) {
		error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
		temp = MAX_SCRIPTS-1;
	}
	if (temp < 0) {
		error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
		temp = 0;
	}
	
	x->current_script = temp;
	rtcmix_dblclick(x);
	
	rtcmix_dprint(x, "rtcmix_openscript() complete");
}


// [setscript N] will set the currently active script to N
void rtcmix_setscript(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_setscript() called");
	
	int i,temp = 0;
	
	if (argc == 0) {
		error("rtcmix~: setscript needs a buffer number [0-19]");
		return;
	}
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				rtcmix_dprint(x, "setscript %i called", temp);
				break;
			case A_FLOAT:
				temp = (int)argv[i].a_w.w_float;
				rtcmix_dprint(x, "setscript %i called", temp);
				break;
		}
	}
	
	if (temp > MAX_SCRIPTS) {
		error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
		temp = MAX_SCRIPTS-1;
	}
	if (temp < 0) {
		error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
		temp = 0;
	}
	
	x->current_script = temp;
	
	rtcmix_dprint(x, "rtcmix_setscript() complete");
}


// the [savescript] message triggers this
void rtcmix_write(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_write() called");
	
	int i, temp = 0;
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				rtcmix_dprint(x, "savescript %i called", temp);
				break;
			case A_FLOAT:
				temp = (int)argv[i].a_w.w_float;
				rtcmix_dprint(x, "savescript %i called", temp);
				break;
		}
	}
	
	if (temp > MAX_SCRIPTS) {
		error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
		temp = MAX_SCRIPTS-1;
	}
	if (temp < 0) {
		error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
		temp = 0;
	}
	
	x->current_script = temp;
	post("rtcmix: current script is %d", temp);
	
	defer(x, (method)rtcmix_dowrite, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_write() complete");
}


// the [savescriptas] message triggers this
void rtcmix_writeas(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_writeas() called");
	
	int i, temp = 0;
	
	for (i=0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				if (temp > MAX_SCRIPTS) {
					error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
					temp = MAX_SCRIPTS-1;
				}
				if (temp < 0) {
					error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
					temp = 0;
				}
				x->current_script = temp;
				x->s_name[x->current_script][0] = 0;
				rtcmix_dprint(x, "savescriptas %i called", temp);
				break;
			case A_FLOAT:
				temp = (int)argv[i].a_w.w_float;
				if (temp > MAX_SCRIPTS) {
					error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
					temp = MAX_SCRIPTS-1;
				}
				if (temp < 0) {
					error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
					temp = 0;
				}
				x->current_script = temp;
				x->s_name[x->current_script][0] = 0;
				rtcmix_dprint(x, "savescriptas %i called", temp);
				break;
			case A_SYM://this doesn't work yet
				strncpy(x->s_name[x->current_script], argv[i].a_w.w_sym->s_name, 256);
				post("rtcmix~: writing file %s",x->s_name[x->current_script]);
		}
	}
	post("rtcmix: current script is %d", temp);
	
	defer(x, (method)rtcmix_dowrite, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_writeas() complete");
}


// deferred from the [save*] messages
void rtcmix_dowrite(t_rtcmix *x, Symbol *s, short argc, t_atom *argv)
{
	rtcmix_dprint(x, "rtcmix_dowrite() called");
	
	char filename[256];
	t_handle script_handle;
	short err;
	t_fourcc type_chosen, thistype = 'TEXT';
	t_filehandle fh;
	
	if(!x->s_name[x->current_script][0]) {
		//if (saveas_dialog(&x->s_name[0][x->current_script], &x->path[x->current_script], &type))
		if (saveasdialog_extended(x->s_name[x->current_script], &x->path[x->current_script], &type_chosen, &thistype, 1))
			return; //user cancelled
	}
	strcpy(filename, x->s_name[x->current_script]);
	
	err = path_createsysfile(filename, x->path[x->current_script], thistype, &fh);
	if (err) {
		fh = 0;
		error("rtcmix~: error %d creating file", err);
		return;
	}
	
	script_handle = sysmem_newhandle(0);
	sysmem_ptrandhand (x->rtcmix_script[x->current_script], script_handle, x->rtcmix_script_len[x->current_script]);
	
	err = sysfile_writetextfile(fh, script_handle, TEXT_LB_UNIX);
	if (err) {
		fh = 0;
		error("rtcmix~: error %d writing file", err);
		return;
	}
	
	// BGG for some reason mach-o doesn't like this one... the memory hit should be small
	//	sysmem_freehandle(script_handle);
	sysfile_seteof(fh, x->rtcmix_script_len[x->current_script]);
	sysfile_close(fh);
	
	rtcmix_dprint(x, "rtcmix_dowrite() complete");
	
	return;
}

// the [read ...] message triggers this
void rtcmix_read(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_read() called");
	
	defer(x, (method)rtcmix_doread, s, argc, argv); // always defer this message
	
	rtcmix_dprint(x, "rtcmix_read() complete");
}

// the deferred read
void rtcmix_doread(t_rtcmix *x, Symbol *s, short argc, t_atom *argv)
{
	rtcmix_dprint(x, "rtcmix_doread() called");
	
	char filename[256];
	short err, i, temp = 0;
	t_fourcc type = 'TEXT';
	t_ptr_size size;
	t_fourcc outtype;
	t_filehandle fh;
	t_handle script_handle;
	
	for (i = 0; i < argc; i++) {
		switch (argv[i].a_type) {
			case A_LONG:
				temp = (int)argv[i].a_w.w_long;
				if (temp > MAX_SCRIPTS) {
					error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
					temp = MAX_SCRIPTS-1;
				}
				if (temp < 0) {
					error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
					temp = 0;
				}
				x->current_script = temp;
				x->s_name[x->current_script][0] = 0;
				break;
			case A_FLOAT:
				if (temp > MAX_SCRIPTS) {
					error("rtcmix~: only %d scripts available, setting to script number %d", MAX_SCRIPTS, MAX_SCRIPTS-1);
					temp = MAX_SCRIPTS-1;
				}
				if (temp < 0) {
					error("rtcmix~: the script number should be > 0!  Resetting to script number 0");
					temp = 0;
				}
				x->current_script = temp;
				temp = (int)argv[i].a_w.w_float;
				x->s_name[x->current_script][0] = 0;
				break;
			case A_SYM:
				strcpy(filename, argv[i].a_w.w_sym->s_name);
				strcpy(x->s_name[x->current_script], filename);
		}
	}
	
	
	if(!x->s_name[x->current_script][0]) {
		//		if (open_dialog(filename, &path, &outtype, &type, 1))
		if (open_dialog(filename,  &x->path[x->current_script], &outtype, 0L, 0)) // allow all types of files
			
			return; //user cancelled
	} else {
		if (locatefile_extended(filename, &x->path[x->current_script], &outtype, &type, 1)) {
			error("rtcmix~: error opening file: can't find file");
			x->s_name[x->current_script][0] = 0;
			return; //not found
		}
	}
	
	//we should have a valid filename at this point
	err = path_opensysfile(filename, x->path[x->current_script], &fh, READ_PERM);
	if (err) {
		fh = 0;
		error("error %d opening file", err);
		return;
	}
	
	strcpy(x->s_name[x->current_script], filename);
	
	sysfile_geteof(fh, &size);
	if (x->rtcmix_script[x->current_script]) {
		freemem((void *)x->rtcmix_script[x->current_script]);
		x->rtcmix_script[x->current_script] = 0;
	}
	// BGG size+1 in max5 to add the terminating '\0'
	if (!(x->rtcmix_script[x->current_script] = (char *)newmem(size+1)) || !(script_handle = sysmem_newhandle(size+1))) {
		error("rtcmix~: %s too big to read", filename);
		return;
	} else {
		x->rtcmix_script_len[x->current_script] = size;
		sysfile_readtextfile(fh, script_handle, size, TEXT_LB_NATIVE);
		strcpy(x->rtcmix_script[x->current_script], *script_handle);
	}
	x->rtcmix_script[x->current_script][size] = '\0'; // the max5 text editor apparently needs this
	// BGG for some reason mach-o doesn't like this one... the memory hit should be small
	//	sysmem_freehandle(*script_handle);
	sysfile_close(fh);
	
	rtcmix_dprint(x, "rtcmix_doread() complete");
	return;
}


// this converts the current script to a binbuf format and sets it up to be saved and then restored
// via the rtcmix_restore() method below
#define RTCMIX_BINBUF_SIZE 5000
void rtcmix_save(t_rtcmix *x, void *w)
{
	rtcmix_dprint(x, "rtcmix_save() called");
	
	char *fptr, *tptr;
	char tbuf[RTCMIX_BINBUF_SIZE]; // max 5's limit on symbol size is 32k, this is totally arbitrary on my part
	int i,j,k;
	
	
	// insert the command to recreate the rtcmix~ object, with any additional vars
	binbuf_vinsert(w, "ssll", gensym("#N"), gensym("rtcmix~"), x->num_inputs, x->num_pinlets);
	
	for (i = 0; i < MAX_SCRIPTS; i++) {
		if (x->rtcmix_script[i] && (x->rtcmix_script_len[i] > 0)) { // there is a script...
			// the reason I do this 'chunking' of restore messages is because of the 32k limit
			// I still wish Cycling had a generic, *untouched* buffer type.
			fptr = x->rtcmix_script[i];
			tptr = tbuf;
			k = 0;
			for (j = 0; j < x->rtcmix_script_len[i]; j++) {
				*tptr++ = *fptr++;
				if (++k >= RTCMIX_BINBUF_SIZE-1) { // 'serialize' the script
					// the 'restore' message contains script #, current buffer length, final buffer length, symbol with buffer contents
					*tptr = '\0';
					binbuf_vinsert(w, "ssllls", gensym("#X"), gensym("restore"), i, k, x->rtcmix_script_len[i], gensym(tbuf));
					tptr = tbuf;
					k = 0;
				}
			}
			// do the final one (or the only one in cases where scripts < 5000)
			*tptr = '\0';
			binbuf_vinsert(w, "ssllls", gensym("#X"), gensym("restore"), i, k, x->rtcmix_script_len[i], gensym(tbuf));
		}
	}
	rtcmix_dprint(x, "rtcmix_save() complete");
}


// and this gets the message set up by rtcmix_save()
void rtcmix_restore(t_rtcmix *x, Symbol *s, short argc, Atom *argv)
{
	rtcmix_dprint(x, "rtcmix_restore() called");
	
	int i;
	int bsize, fsize;
	char *fptr; // restore buf pointer is in the struct for repeated calls necessary for larger scripts (symbol size limit, see rtcmix_save())
	
	// script #, current buffer size, final script size, script data
	x->current_script = argv[0].a_w.w_long;
	bsize = argv[1].a_w.w_long;
	if (argv[2].a_type == A_SYM) { // this is for v1.399 scripts that had the symbol size limit bug
		fsize = bsize;
		fptr = argv[2].a_w.w_sym->s_name;
	} else {
		fsize = argv[2].a_w.w_long;
		fptr = argv[3].a_w.w_sym->s_name;
	}
	
	if (!x->rtcmix_script[x->current_script]) { // if the script isn't being restored already
		if (!(x->rtcmix_script[x->current_script] = (char *)newmem(fsize+1))) { // fsize+1 for the '\0
			error("rtcmix~: problem allocating memory for restored script");
			return;
		}
		x->rtcmix_script_len[x->current_script] = fsize;
		x->restore_buf_ptr = x->rtcmix_script[x->current_script];
	}
	
	// this happy little for-loop is for older (max 4.x) rtcmix scripts.  The older version of max had some
	// serious parsing issues for saved text.  Now it all seems fixed in 5 -- yay!
	// convert the xRTCMIX_XXx tokens to their real equivalents
	// I'm not sure the above (fixed in max5) is actually true, gonna keep this
	for (i = 0; i < bsize; i++) {
		switch (*fptr) {
			case 'x':
				if (strncmp(fptr, "xRTCMIX_CRx", 11) == 0) {
					sprintf(x->restore_buf_ptr, "\r");
					fptr += 11;
					x->restore_buf_ptr++;
					break;
				}
				else if (strncmp(fptr, "xRTCMIX_DQx", 11) == 0) {
					sprintf(x->restore_buf_ptr, "\"");
					fptr += 11;
					x->restore_buf_ptr++;
					break;
				} else {
					*x->restore_buf_ptr++ = *fptr++;
					break;
				}
			default:
				*x->restore_buf_ptr++ = *fptr++;
		}
	}
	
	x->rtcmix_script[x->current_script][fsize] = '\0'; // the final '\0'
	
	x->current_script = 0; // do this to set script 0 as default
	
	rtcmix_dprint(x, "rtcmix_restore() complete");
}
