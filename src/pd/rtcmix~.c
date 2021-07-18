#include "m_pd.h"
#include "m_imp.h"
#include "rtcmix~.h"
//#include "../rtcmix/RTcmix_API.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>


#define UNUSED(x) (void)(x)

#define DEBUG(x) // debug off
//#define DEBUG(x) x // debug on

#ifdef MACOSX
#define OS_OPENCMD "open"
#else
#define OS_OPENCMD "xdg-open"
#endif

void rtcmix_tilde_setup(void)
{
        rtcmix_tilde_class = class_new (gensym("rtcmix~"),
                                        (t_newmethod)rtcmix_tilde_new,
                                        (t_method)rtcmix_tilde_free,
                                        sizeof(t_rtcmix_tilde),
                                        0, A_GIMME, 0);

        //standard messages; don't change these
        CLASS_MAINSIGNALIN(rtcmix_tilde_class, t_rtcmix_tilde, f);
        class_addmethod(rtcmix_tilde_class, (t_method)rtcmix_tilde_dsp, gensym("dsp"), 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_info, gensym("info"), 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_reference, gensym("reference"), 0);
        class_addbang(rtcmix_tilde_class, rtcmix_tilde_bang); // trigger scripts
        class_addfloat(rtcmix_tilde_class, rtcmix_tilde_float);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_openeditor, gensym("click"), 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_editor, gensym("editor"), A_SYMBOL, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_var, gensym("var"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_varlist, gensym("varlist"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_flush, gensym("flush"), 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_reset, gensym("reset"), 0);

        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_open, gensym("read"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_open, gensym("load"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_open, gensym("open"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_save, gensym("write"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_save, gensym("save"), A_GIMME, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_save, gensym("saveas"), A_GIMME, 0);
        // openpanel and savepanel return their messages through "callback"
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_callback, gensym("callback"), A_SYMBOL, 0);

        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_setscript, gensym("setscript"), A_FLOAT, 0);
        class_addmethod(rtcmix_tilde_class,(t_method)rtcmix_bufset, gensym("bufset"), A_SYMBOL, 0);
    
    // BGG -- clear out /tmp of any RTcmix_* dylib temp folders left behind from crashes, etc.
    // see note below on why these are here
    system("rm -rf /tmp/RTcmix_*");
    
    post("rtcmix~: RTcmix music language, http://rtcmix.org");
    post("rtcmix~: version %s, compiled at %s on %s",VERSION,__TIME__, __DATE__);
}

void *rtcmix_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
        t_rtcmix_tilde *x = (t_rtcmix_tilde *)pd_new(rtcmix_tilde_class);
        UNUSED(s);
        null_the_pointers(x);

        char *sys_cmd = malloc(MAXPDSTRING);
        char* externdir = (char *)rtcmix_tilde_class->c_externdir->s_name;
        x->libfolder = malloc(MAXPDSTRING);
        sprintf(x->libfolder,"%s/lib", externdir);
        externdir = NULL;
        DEBUG(post("rtcmix~: libfolder: %s", x->libfolder); );
        x->tempfolder = malloc(MAXPDSTRING);
        sprintf(x->tempfolder,"/tmp/RTcmix_XXXXXX");

        // create temp folder
// BGG -- include the mkdtemp() definition here to prevent a warning
char *mkdtemp(char *);

// The way this works:  because dlopen() will only open on instance of
// a loadable lib, and we need a separate one for each rtcmix~ object,
// the lib gets copied with a unique name to the /tmp/folder and then
// gets loaded from there.  We clean up on exit and in the free() function
// Also this /tmp/RTcmix_XXXX dir us used to stash newly-opened script
// scorefiles for the editor.
// I had done a version of this in the first few iterations of rtcmix~
// for max/msp, but discovered in OSX that the NSLinkModule() method
// does what I want.
        void* pointless_pointer = mkdtemp(x->tempfolder);
        UNUSED(pointless_pointer);

        // create unique name for dylib
        x->dylib = malloc(MAXPDSTRING);
        char template[MAXPDSTRING];
        sprintf(template, "%s/RTcmix_dylib_XXXXXX", x->tempfolder);
        if (!mkstemp(template)) error ("failed to create dylib temp name");
        sprintf(x->dylib,"%s", template);
        DEBUG(post("rtcmix~: tempfolder: %s, dylib: %s", x->tempfolder, x->dylib); );
        // allow other users to read and write (no execute tho)
        sprintf(sys_cmd, "chmod 766 %s", x->tempfolder);
        if (system(sys_cmd))
                error ("rtcmix~: error setting temp folder \"%s\" permissions.", x->tempfolder);
        sprintf(sys_cmd, "cp %s/%s %s", x->libfolder, DYLIBNAME, x->dylib);
        if (system(sys_cmd))
                error ("rtcmix~: error copying dylib");
    
        x->editorpath = malloc(MAXPDSTRING);
    // BGG see my notes in rtcmix_openeditor() below
#ifdef MACOSX
        sprintf(x->editorpath, "python \"%s/%s\"", x->libfolder, "rtcmix_editor.py");
#else
        sprintf(x->editorpath, "tclsh \"%s/%s\"", x->libfolder, "tedit");
#endif

        free(sys_cmd);

        unsigned int num_inoutputs = 1;
        unsigned int num_additional = 0;
        // JWM: add optional third argument to autoload scorefile
        t_symbol* optional_filename = NULL;
        unsigned int float_arg = 0;
        for (short this_arg=0; this_arg<argc; this_arg++)
        {
                switch (argv[this_arg].a_type)
                {
                case A_SYMBOL:
                        optional_filename = argv[this_arg].a_w.w_symbol;
                        DEBUG( post("rtcmix~: instantiating with scorefile %s",optional_filename->s_name); );
                        break;
                case A_FLOAT:
                        if (float_arg == 0)
                        {
                                num_inoutputs = atom_getint(argv+this_arg);
                                DEBUG(post("rtcmix~: creating with %d signal inlets and outlets", num_inoutputs); );
                                float_arg++;
                        }
                        else if (float_arg == 1)
                        {
                                num_additional = atom_getint(argv+this_arg);
                                DEBUG(post("rtcmix~: creating with %d pfield inlets", num_additional); );
                        }
                default:
                {}
                }
        }
        //DEBUG(post("creating %d inlets and outlets and %d additional inlets",num_inoutputs,num_additional););
        if (num_inoutputs < 1) num_inoutputs = 1; // no args, use default of 1 channel in/out

        // JWM: limiting num_additional to 10
        if (num_additional > MAX_PINLETS)
        {
                error ("rtcmix~: sorry, only %d pfield inlets are allowed", MAX_PINLETS);
                num_additional = MAX_PINLETS;
        }

        if (num_inoutputs > MAX_INPUTS)
        {
                num_inoutputs = MAX_INPUTS;
                error("rtcmix~: sorry, only %d signal inlets are allowed", MAX_INPUTS);
        }

        x->num_inputs = num_inoutputs;
        x->num_outputs = num_inoutputs;
        x->num_pinlets = num_additional;

        // setup up inputs and outputs, for audio inputs

        //x->signal_inlets = malloc (x->num_inputs);
        x->vector_size = 0;
        // SIGNAL INLETS
        for (int i=0; i < x->num_inputs-1; i++)
                inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

        x->pfield_in = malloc(x->num_pinlets);
        //x->pfield_inlets = malloc(x->num_pinlets);
        for (short i=0; i< x->num_pinlets; i++)
        {
                x->pfield_in[i] = 0.0;
                floatinlet_new(&x->x_obj, &x->pfield_in[i]);
        }

        // SIGNAL OUTLETS
        //x->signal_outlets = malloc(x->num_outputs);
        for (int i = 0; i < x->num_outputs; i++)
        {
                // outputs, right-to-left
                outlet_new(&x->x_obj, gensym("signal"));
        }

        // OUTLET FOR BANGS
        x->outpointer = outlet_new(&x->x_obj, &s_bang);

        // set up for the variable-substitution scheme
        x->var_array = malloc(NVARS * MAXPDSTRING);
        for(short i = 0; i < NVARS; i++)
        {
                x->var_array[i] = malloc (MAXPDSTRING);
                sprintf(x->var_array[i], "%g", 0.);
        }
        // the text editor

        char buf[50];
        sprintf(buf, "d%lx", (t_int)x);
        x->x_s = gensym(buf);
        x->x_canvas = canvas_getcurrent();
        x->canvas_path = canvas_getdir(x->x_canvas);
        pd_bind(&x->x_obj.ob_pd, x->x_s);

        DEBUG(post("rtcmix~: editor_path: %s", x->editorpath); );

        x->current_script = 0;
        x->rtcmix_script = malloc(MAX_SCRIPTS * MAXSCRIPTSIZE);
        x->script_path = malloc(MAX_SCRIPTS * MAXPDSTRING);
        x->vars_present = false; // for $ variables

        for (short i=0; i<MAX_SCRIPTS; i++)
        {
                x->rtcmix_script[i] = malloc(MAXSCRIPTSIZE);
                x->script_path[i] = malloc(MAXPDSTRING);
                sprintf(x->script_path[i],"%s/%s%i.%s",x->tempfolder, TEMPFILENAME, i, SCOREEXTENSION);
                //DEBUG(post(x->script_path[i],"%s/%s%i.%s",x->tempfolder, TEMPFILENAME, i, SCOREEXTENSION););
        }

        x->flushflag = false;
        x->rw_flag = none;

        // If filename is given in score instantiation, open scorefile
        if (optional_filename)
        {
                char* fullpath = malloc(MAXPDSTRING);
                canvas_makefilename(x->x_canvas, optional_filename->s_name, fullpath, MAXPDSTRING);
                sprintf(x->script_path[x->current_script], "%s", fullpath);
                DEBUG( post ("opening scorefile %s", optional_filename->s_name); );
                //post("default scorefile: %s", default_scorefile->s_name);
                rtcmix_read(x, fullpath);
                free(fullpath);
        }
        x->rw_flag = none;
        x->buffer_changed = false;
        return (void *)x;
}

void rtcmix_tilde_dsp(t_rtcmix_tilde *x, t_signal **sp)
{
        // This is 2 because (for some totally crazy reason) the
        // signal vector starts at 1, not 0
        t_int dsp_add_args [x->num_inputs + x->num_outputs + 2];
        x->vector_size = sp[0]->s_n;
        x->srate = sys_getsr();

        // construct the array of vectors and stuff
        dsp_add_args[0] = (t_int)x; //the object itself
        for(int i = 0; i < (x->num_inputs + x->num_outputs); i++)
        { //pointers to the input and output vectors
                dsp_add_args[i+1] = (t_int)sp[i]->s_vec;
        }

        dsp_add_args[x->num_inputs + x->num_outputs + 1] = x->vector_size; //pointer to the vector size

        DEBUG(post("vector size: %d",x->vector_size); );

        dsp_addv(rtcmix_tilde_perform, (x->num_inputs  + x->num_outputs + 2),(t_int*)dsp_add_args);//add them to the signal chain

        // load the dylib
        dlopen_and_errorcheck(x);
    
        // BGG -- for some reason this is necessary for OSX to reset the bus_configs
        x->RTcmix_destroy();
    
        x->RTcmix_init();
        x->RTcmix_setBangCallback(rtcmix_bangcallback, x);
        x->RTcmix_setValuesCallback(rtcmix_valuescallback, x);
        x->RTcmix_setPrintCallback(rtcmix_printcallback, x);

// allocate the RTcmix i/o transfer buffers
        x->pd_inbuf = malloc(sizeof(float) * x->vector_size * x->num_inputs);
        x->pd_outbuf = malloc(sizeof(float) * x->vector_size * x->num_outputs);

        // zero out these buffers for UB
        //for (int i = 0; i < (x->vector_size * x->num_inputs); i++) x->pd_inbuf[i] = 0.0;
        //for (int i = 0; i < (x->vector_size * x->num_outputs); i++) x->pd_outbuf[i] = 0.0;

        DEBUG(post("x->srate: %f, x->num_outputs: %d, x->vector_size %d, 1, 0", x->srate, x->num_outputs, x->vector_size); );
        x->RTcmix_setAudioBufferFormat(AudioFormat_32BitFloat_Normalized, x->num_outputs);
        x->RTcmix_setparams(x->srate, x->num_outputs, x->vector_size, true, 0);
}

t_int *rtcmix_tilde_perform(t_int *w)
{
        t_rtcmix_tilde *x = (t_rtcmix_tilde *)(w[1]);
        t_int vecsize = w[x->num_inputs + x->num_outputs + 2]; //number of samples per vector
        float *in[x->num_inputs * vecsize]; //pointers to the input vectors
        float *out[x->num_outputs * vecsize]; //pointers to the output vectors

        //int i = x->num_outputs * vecsize;
        //while (i--) out[i] = (float *)0.;

        x->checkForBang();
        x->checkForVals();
        x->checkForPrint();

        for (int i = 0; i < x->num_pinlets; i++)
        {
                x->RTcmix_setPField(i, x->pfield_in[i]);
        }

        // reset queue and heap if signalled
        if (x->flushflag == true)
        {
                x->RTcmix_flushScore();
                x->flushflag = false;
        }


        for (int i = 0; i < x->num_inputs; i++)
        {
                in[i] = (float *)(w[i+2]);
        }

        //assign the output vectors
        for (int i = 0; i < x->num_outputs; i++)
        {
                // this results in reversed L-R image but I'm
                // guessing it's the same in Max
                out[i] = (float *)( w[x->num_inputs + i + 2 ]);
        }

        int j = 0;
        for (int k = 0; k < vecsize; k++)
        {
                for (int i = 0; i < x->num_inputs; i++)
                        (x->pd_inbuf)[j++] = *in[i]++;
        }

        x->RTcmix_runAudio (x->pd_inbuf, x->pd_outbuf, 1);

        j = 0;
        for (int k = 0; k < vecsize; k++)
        {
                for(int i = 0; i < x->num_outputs; i++)
                        *out[i]++ = (x->pd_outbuf)[j++];
        }

        return w + x->num_inputs + x->num_outputs + 3;
}

void rtcmix_tilde_free(t_rtcmix_tilde *x)
{
        if (x->RTcmix_flushScore) x->RTcmix_flushScore();
    
    // BGG -- remove the RTcmix_XXX folder in /tmp
    char rmtmpfoldercmd[512];
    sprintf(rmtmpfoldercmd, "rm -rf %s", x->tempfolder);
    system(rmtmpfoldercmd);
    
        free(x->tempfolder);
        free(x->pd_inbuf);
        free(x->pd_outbuf);
        for (int i = 0; i < NVARS; i++)
                free(x->var_array[i]);
        free(x->var_array);
        //free(x->x_s);
        free(x->editorpath);

        for (short i=0; i<MAX_SCRIPTS; i++)
        {
                free(x->rtcmix_script[i]);
                free(x->script_path[i]);
        }
        free (x->rtcmix_script);
        free (x->script_path);
        outlet_free(x->outpointer);
        /*
           for (int i = 0; i < x->num_inputs; i++)
                inlet_free (x->signal_inlets[i]);
           for (int i = 0; i < x->num_pinlets; i++)
                inlet_free (x->pfield_inlets[i]);
           for (int i = 0; i < x->num_outputs; i++)
                outlet_free (x->signal_outlets[i]);
         */
        //x->canvas_path = NULL;
        //x->x_canvas = NULL;

        if (x->RTcmix_dylib)
        {
                x->RTcmix_destroy();
                dlclose(x->RTcmix_dylib);
                x->dylib = NULL;
        }
    
        DEBUG(post ("rtcmix~ DESTROYED!"); );
}

// BGG -- the free() function above doesn't get called if you simply
// quit pd with an open rtcmix~ object in a patch, so this gets called
// on quit and cleans it all up.  I could just do this I supposed and
// not worry about removing the RTcmix_* files in /tmp.
__attribute__((destructor))
static void cleantmp() {
	system("rm -rf /tmp/RTcmix_*");
}

void rtcmix_info(t_rtcmix_tilde *x)
{
        post("rtcmix~, v. %s by Joel Matthys (minor alterations by Brad Garton)", VERSION);
        post("compiled at %s on %s",__TIME__, __DATE__);
        post("temporary files are located at %s", x->tempfolder);
        post("temporary rtcmixdylib.so is %s", x->dylib);
        post("rtcmix~ external is located at %s", x->canvas_path->s_name);
        post("current scorefile is %s", x->script_path[x->current_script]);
        post("using editor %s", x->editorpath);
        outlet_bang(x->outpointer);
}

// the "var" message allows us to set $n variables imbedded in a scorefile with varnum value messages
void rtcmix_var(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);

        for (int i = 0; i < argc; i += 2)
        {
                unsigned int varnum = (unsigned int)argv[i].a_w.w_float;
                if ( (varnum < 1) || (varnum > NVARS) )
                {
                        error("only vars $1 - $9 are allowed");
                        return;
                }
                if (argv[i].a_type == A_SYMBOL)
                        sprintf(x->var_array[varnum-1], "%s", argv[i+1].a_w.w_symbol->s_name);
                else if (argv[i].a_type == A_FLOAT)
                        sprintf(x->var_array[varnum-1], "%g", argv[i+1].a_w.w_float);
        }
        DEBUG(post("vars: %s %s %s %s %s %s %s %s %s",
                   x->var_array[0],
                   x->var_array[1],
                   x->var_array[2],
                   x->var_array[3],
                   x->var_array[4],
                   x->var_array[5],
                   x->var_array[6],
                   x->var_array[7],
                   x->var_array[8]); );
}


// the "varlist" message allows us to set $n variables imbedded in a scorefile with a list of positional vars
void rtcmix_varlist(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);

        if (argc > NVARS)
        {
                error("asking for too many variables, only setting the first 9 ($1-$9)");
                argc = NVARS;
        }

        for (short i = 0; i < argc; i++)
        {
                if (argv[i].a_type == A_SYMBOL)
                        sprintf(x->var_array[i], "%s", argv[i].a_w.w_symbol->s_name);
                else if (argv[i].a_type == A_FLOAT)
                        sprintf(x->var_array[i], "%g", argv[i].a_w.w_float);

        }
        DEBUG(post("vars: %s %s %s %s %s %s %s %s %s",
                   x->var_array[0],
                   x->var_array[1],
                   x->var_array[2],
                   x->var_array[3],
                   x->var_array[4],
                   x->var_array[5],
                   x->var_array[6],
                   x->var_array[7],
                   x->var_array[8]); );
}

// the "flush" message
void rtcmix_flush(t_rtcmix_tilde *x)
{
        DEBUG( post("flushing"); );
        if (canvas_dspstate == 0) return;
        if (x->RTcmix_flushScore) x->RTcmix_flushScore();
}

// bang triggers the current working script
void rtcmix_tilde_bang(t_rtcmix_tilde *x)
{
        DEBUG(post("rtcmix~: received bang"); );
        //if (x->flushflag == true) return; // heap and queue being reset
        if (canvas_dspstate == 0)
        {
                error ("rtcmix~: can't interpret scorefile until DSP is on.");
                return;
        }
        post("rtcmix~: playing \"%s\"", x->script_path[x->current_script]);
        if (x->buffer_changed) rtcmix_read(x, x->script_path[x->current_script]);
        if (x->vars_present) sub_vars_and_parse(x, x->rtcmix_script[x->current_script]);
        else x->RTcmix_parseScore(x->rtcmix_script[x->current_script], strlen(x->rtcmix_script[x->current_script]));
}

void rtcmix_tilde_float(t_rtcmix_tilde *x, t_float scriptnum)
{
        DEBUG(post("received float %f",scriptnum); );
        if (x->flushflag == true) return; // heap and queue being reset
        if (canvas_dspstate == 0)
        {
                error ("rtcmix~: can't interpret scorefile until DSP is on.");
                return;
        }
        if (scriptnum < 0 || scriptnum >= MAX_SCRIPTS)
        {
                error ("%d is not a valid script number. (Must be 0 - %d)", (int)scriptnum, (int)(MAX_SCRIPTS-1));
                return;
        }
        post("rtcmix~: playing \"%s\"", x->script_path[(int)scriptnum]);
        rtcmix_read(x, x->script_path[(int)scriptnum]);
        sub_vars_and_parse(x, x->rtcmix_script[(int)scriptnum]);
}

void rtcmix_openeditor(t_rtcmix_tilde *x)
{
        DEBUG( post ("clicked."); );
        x->buffer_changed = true;
        DEBUG( post("x->script_path[x->current_script]: %s", x->script_path[x->current_script]); );

// BGGx
// The following command (for the python editor) does not work on my generic
// 10.13.6 High Sierra mac.  The python editor will work when invoked
// by a system() call, though, so I'm using that for now

//        sys_vgui("exec %s %s &\n",x->editorpath, x->script_path[x->current_script]);

		char editorcommand[512];
		sprintf(editorcommand, "%s %s", x->editorpath, x->script_path[x->current_script]);
		system(editorcommand);

// However, I can invoke TextEdit using the sys_vgui() approach and it
// works (will have to pre-create a file for empty scores, though)
// I'm leaving it here in case it is necessary for a Windows editor

//		sys_vgui("exec /usr/bin/open -a TextEdit %s &\n", x->script_path[x->current_script]);
}

void rtcmix_editor (t_rtcmix_tilde *x, t_symbol *s)
{
        char *str = (char *)s->s_name;
        if (0==strcmp(str,"tedit")) sprintf(x->editorpath, "\"%s/%s\"", x->libfolder, "tedit");
        else if (0==strcmp(str,"rtcmix")) sprintf(x->editorpath, "python \"%s/%s\"", x->libfolder, "rtcmix_editor.py");
        else sprintf(x->editorpath, "\"%s\"", str);
        post("rtcmix~: setting the text editor to %s",str);
}

void rtcmix_setscript(t_rtcmix_tilde *x, t_float s)
{
        DEBUG(post("setscript: %d", (int)s); );
        if (s >= 0 && s < MAX_SCRIPTS)
        {
                DEBUG(post ("changed current script to %d", (int)s); );
                x->current_script = (int)s;
        }
        else error ("%d is not a valid script number. (Must be 0 - %d)", (int)s, (int)(MAX_SCRIPTS-1));
}

void rtcmix_read(t_rtcmix_tilde *x, char* fullpath)
{
        DEBUG( post("read %s",fullpath); );

        char *buffer = malloc(MAXSCRIPTSIZE);
        buffer = ReadFile(fullpath);
        int lSize = strlen(buffer);

        if (lSize>MAXSCRIPTSIZE)
        {
                error("rtcmix~: read error: file is longer than MAXSCRIPTSIZE");
                return;
        }

        if (lSize==0)
        {
                error("rtcmix~: read error: file is length 0");
                return;
        }
        sprintf(x->rtcmix_script[x->current_script], "%s",buffer);
        sprintf(x->script_path[x->current_script], "%s", fullpath);

        x->vars_present = false;
        for (int i=0; i<lSize; i++) if ((int)buffer[i]==36) x->vars_present = true;
        x->buffer_changed = false;

}

void rtcmix_write(t_rtcmix_tilde *x, char* filename)
{
        DEBUG (post("write %s", filename); );
        char * sys_cmd = malloc(MAXPDSTRING);
        sprintf(sys_cmd, "cp \"%s\" \"%s\"", x->script_path[x->current_script], filename);
        if (system(sys_cmd)) error ("rtcmix~: error saving %s",filename);
        else
        {
                DEBUG(post("rtcmix~: wrote script %i to %s",x->current_script,filename); );
        }
        sprintf(x->script_path[x->current_script], "%s", filename);
        free(sys_cmd);
}

void rtcmix_open(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);
        if (argc == 0)
        {
                x->rw_flag = read;
                DEBUG( post ("pdtk_openpanel {%s} {%s}\n", x->x_s->s_name, x->canvas_path->s_name); );
                sys_vgui("pdtk_openpanel {%s} {%s}\n", x->x_s->s_name, x->canvas_path->s_name);
        }
        else
        {
                char* fullpath = malloc(MAXPDSTRING);
                canvas_makefilename(x->x_canvas, argv[0].a_w.w_symbol->s_name, fullpath, MAXPDSTRING);
                rtcmix_read(x, fullpath);
                free (fullpath);
        }
}

void rtcmix_save(t_rtcmix_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);
        if (argc == 0)
        {
                x->rw_flag = write;
                DEBUG( post ("pdtk_savepanel {%s} {%s}\n", x->x_s->s_name, x->canvas_path->s_name); );
                sys_vgui("pdtk_savepanel {%s} {%s}\n", x->x_s->s_name, x->canvas_path->s_name);
        }
        else
        {
                char* fullpath = malloc(MAXPDSTRING);
                canvas_makefilename(x->x_canvas, argv[0].a_w.w_symbol->s_name, fullpath, MAXPDSTRING);
                rtcmix_write(x, fullpath);
                free (fullpath);
        }
}

void rtcmix_callback (t_rtcmix_tilde *x, t_symbol *s)
{
        switch (x->rw_flag)
        {
        case read:
                //post("read %s", s->s_name);
                rtcmix_read(x, (char *)s->s_name);
                break;
        case write:
                //post("write %s", s->s_name);
                rtcmix_write(x, (char *)s->s_name);
                break;
        case none:
        default:
                error("callback error: %s", s->s_name);
        }
        x->rw_flag = none;
}

void rtcmix_bangcallback(void *inContext)
{
        t_rtcmix_tilde *x = (t_rtcmix_tilde *) inContext;
        // got a pending bang from MAXBANG()
        outlet_bang(x->outpointer);
}

void rtcmix_valuescallback(float *values, int numValues, void *inContext)
{
        t_rtcmix_tilde *x = (t_rtcmix_tilde *) inContext;
        // BGG -- I should probably defer this one and the error posts also.  So far not a problem...
        DEBUG(post("numValues: %d", numValues); );
        if (numValues == 1)
                outlet_float(x->outpointer, (double)(values[0]));
        else {
                for (short i = 0; i < numValues; i++) SETFLOAT((x->valslist)+i, values[i]);
                outlet_list(x->outpointer, 0L, numValues, x->valslist);
        }
}

void rtcmix_printcallback(const char *printBuffer, void *inContext)
{
        UNUSED(inContext);
        const char *pbufptr = printBuffer;
        while (strlen(pbufptr) > 0) {
                post("RTcmix: %s", pbufptr);
                pbufptr += (strlen(pbufptr) + 1);
        }
}

void null_the_pointers(t_rtcmix_tilde *x)
{
        x->pfield_in = NULL;
        x->outpointer = NULL;
        x->tempfolder = NULL;
        x->pd_inbuf = NULL;
        x->pd_outbuf = NULL;
        x->var_array = NULL;
        x->rtcmix_script = NULL;
        x->script_path = NULL;
        x->x_canvas = NULL;
        x->canvas_path = NULL;
        x->x_s = NULL;
        x->editorpath = NULL;
        x->tempfolder = NULL;
        x->dylib = NULL;
        x->libfolder = NULL;
        x->RTcmix_dylib = NULL;
        x->RTcmix_init = NULL;
        x->RTcmix_destroy = NULL;
        x->RTcmix_setparams = NULL;
        x->RTcmix_setBangCallback = NULL;
        x->RTcmix_setValuesCallback = NULL;
        x->RTcmix_setPrintCallback = NULL;
        x->RTcmix_resetAudio = NULL;
        x->RTcmix_setAudioBufferFormat = NULL;
        x->RTcmix_runAudio = NULL;
        x->RTcmix_parseScore = NULL;
        x->RTcmix_flushScore = NULL;
        x->RTcmix_setInputBuffer = NULL;
        x->RTcmix_getBufferFrameCount = NULL;
        x->RTcmix_getBufferChannelCount = NULL;
        x->RTcmix_setPField = NULL;
        x->checkForBang = NULL;
        x->checkForVals = NULL;
        x->checkForPrint = NULL;
}

void dlopen_and_errorcheck (t_rtcmix_tilde *x)
{
        // reload, this reinits the RTcmix queue, etc.
        if (x->RTcmix_dylib) dlclose(x->RTcmix_dylib);

        x->RTcmix_dylib = dlopen(x->dylib, RTLD_NOW);
        if (!x->RTcmix_dylib)
        {
                error("dlopen error loading dylib");
                error("%s",dlerror());
        }
        x->RTcmix_init = dlsym(x->RTcmix_dylib, "RTcmix_init");
        if (!x->RTcmix_init) error("RTcmix could not call RTcmix_init()");
        x->RTcmix_destroy = dlsym(x->RTcmix_dylib, "RTcmix_destroy");
        if (!x->RTcmix_destroy) error("RTcmix could not call RTcmix_destroy()");
        x->RTcmix_setparams = dlsym(x->RTcmix_dylib, "RTcmix_setparams");
        if (!x->RTcmix_setparams) error("RTcmix could not call RTcmix_setparams()");
        x->RTcmix_setBangCallback = dlsym(x->RTcmix_dylib, "RTcmix_setBangCallback");
        if (!x->RTcmix_setBangCallback) error("RTcmix could not call RTcmix_setBangCallback()");
        x->RTcmix_setValuesCallback = dlsym(x->RTcmix_dylib, "RTcmix_setValuesCallback");
        if (!x->RTcmix_setValuesCallback) error("RTcmix could not call RTcmix_setValuesCallback()");
        x->RTcmix_setPrintCallback = dlsym(x->RTcmix_dylib, "RTcmix_setPrintCallback");
        if (!x->RTcmix_setPrintCallback) error("RTcmix could not call RTcmix_setPrintCallback()");
        x->RTcmix_resetAudio = dlsym(x->RTcmix_dylib, "RTcmix_resetAudio");
        if (!x->RTcmix_resetAudio) error("RTcmix could not call RTcmix_resetAudio()");
        x->RTcmix_setAudioBufferFormat = dlsym(x->RTcmix_dylib, "RTcmix_setAudioBufferFormat");
        if (!x->RTcmix_setAudioBufferFormat) error("RTcmix could not call RTcmix_setAudioBufferFormat()");
        x->RTcmix_runAudio = dlsym(x->RTcmix_dylib, "RTcmix_runAudio");
        if (!x->RTcmix_runAudio) error("RTcmix could not call RTcmix_runAudio()");
        x->RTcmix_parseScore = dlsym(x->RTcmix_dylib, "RTcmix_parseScore");
        if (!x->RTcmix_parseScore) error("RTcmix could not call RTcmix_parseScore()");
        x->RTcmix_flushScore = dlsym(x->RTcmix_dylib, "RTcmix_flushScore");
        if (!x->RTcmix_flushScore) error("RTcmix could not call RTcmix_flushScore()");
        x->RTcmix_setInputBuffer = dlsym(x->RTcmix_dylib, "RTcmix_setInputBuffer");
        if (!x->RTcmix_setInputBuffer) error("RTcmix could not call RTcmix_setInputBuffer()");
        x->RTcmix_getBufferFrameCount = dlsym(x->RTcmix_dylib, "RTcmix_getBufferFrameCount");
        if (!x->RTcmix_getBufferFrameCount) error("RTcmix could not call RTcmix_getBufferFrameCount()");
        x->RTcmix_getBufferChannelCount = dlsym(x->RTcmix_dylib, "RTcmix_getBufferChannelCount");
        if (!x->RTcmix_getBufferChannelCount) error("RTcmix could not call RTcmix_getBufferChannelCount()");
        x->RTcmix_setPField = dlsym(x->RTcmix_dylib, "RTcmix_setPField");
        if (!x->RTcmix_setPField) error("RTcmix could not call RTcmix_setPField()");
        x->checkForBang = dlsym(x->RTcmix_dylib, "checkForBang");
        if (!x->checkForBang) error("RTcmix could not call checkForBang()");
        x->checkForVals = dlsym(x->RTcmix_dylib, "checkForVals");
        if (!x->checkForVals) error("RTcmix could not call checkForVals()");
        x->checkForPrint = dlsym(x->RTcmix_dylib, "checkForPrint");
        if (!x->checkForPrint) error("RTcmix could not call checkForPrint()");
}

void rtcmix_bufset(t_rtcmix_tilde *x, t_symbol *s)
{
        arraynumber_t *vec;
        int vecsize;
        if (canvas_dspstate == 1)
        {
                t_garray *g;
                DEBUG(post("rtcmix~: bufset %s",s->s_name); );
                if ((g = (t_garray *)pd_findbyclass(s,garray_class)))
                {
                        if (!array_getarray(g, &vecsize, &vec))
                        {
                                error("rtcmix~: can't read array");
                        }
                        int chans = sizeof(t_word)/sizeof(float);
                        DEBUG(post("rtcmix~: word size: %d, float size: %d",sizeof(t_word), sizeof(float)); );
                        post("rtcmix~: registered buffer \"%s\"", s->s_name);
                        x->RTcmix_setInputBuffer((char *)s->s_name, (float*)vec, vecsize, chans, 0);
                }
                else
                {
                        error("rtcmix~: no array \"%s\"", s->s_name);
                }
        }
        else
                error ("rtcmix~: can't add buffer with DSP off");
}

void sub_vars_and_parse (t_rtcmix_tilde *x, const char* script)
{
        char* script_out = malloc(MAXSCRIPTSIZE);
        unsigned int scriptsize = strlen(script);
        //post("scriptsize: %d", scriptsize);
//        unsigned int inchar = 0;
        unsigned int outchar = 0;
//        while (inchar < scriptsize)
        for (unsigned int inchar = 0; inchar < scriptsize; inchar++)
        {
                char testchar = script[inchar];
                //post("testchar at position %d: %c", inchar, testchar);
                if ((int)testchar == 0)
                {
                        error ("null character found at position %d", inchar);
                }
                if ((int)testchar != 36) // dollar sign
                        script_out[outchar++] = testchar;
                else // Dollar sign found
                {
                        int varnum = (int)script[inchar+1] - 49;
                        if (varnum < 0 || varnum > 8)
                        {
                                error("rtcmix~: $ variable in script must be followed by a number 1-9");
                        }
                        else
                        {
                                int num_insert_chars = strlen(x->var_array[varnum]);
                                for (int i = 0; i < num_insert_chars; i++)
                                {
                                        script_out[outchar++] = (int)x->var_array[varnum][i];
                                }
                                inchar++; // skip number argument
                        }
                }
                //inchar++;
        }
        x->RTcmix_parseScore(script_out, outchar);
}

void rtcmix_reference(t_rtcmix_tilde *x)
{
        UNUSED(x);
        sys_vgui("exec %s http://rtcmix.org/reference &\n", OS_OPENCMD);
}

void rtcmix_reset(t_rtcmix_tilde *x)
{
        if (canvas_dspstate == 0) return;
        if (x->RTcmix_resetAudio) x->RTcmix_resetAudio(x->srate, x->num_outputs, x->vector_size, 1);
}

char* ReadFile(char *filename)
{
        char *buffer = NULL;
        int string_size, read_size;
        FILE *handler = fopen(filename, "r");

        if (handler)
        {
                // Seek the last byte of the file
                fseek(handler, 0, SEEK_END);
                // Offset from the first to the last byte, or in other words, filesize
                string_size = ftell(handler);
                // go back to the start of the file
                rewind(handler);

                // Allocate a string that can hold it all
                buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

                // Read it all in one operation
                read_size = fread(buffer, sizeof(char), string_size, handler);

                // fread doesn't set it so put a \0 in the last position
                // and buffer is now officially a string
                buffer[string_size] = '\0';

                if (string_size != read_size)
                {
                        // Something went wrong, throw away the memory and set
                        // the buffer to NULL
                        free(buffer);
                        buffer = NULL;
                }

                // Always remember to close the file.
                fclose(handler);
        }

        return buffer;
}
