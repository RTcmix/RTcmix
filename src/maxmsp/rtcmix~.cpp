#include <c74_min.h>
#include <vector>
#include <dlfcn.h>

// Hand-defining these here because these will always be true, and are needed to get the right portions of RTcmix_API.h
#define EMBEDDEDAUDIO 1
#define MAXMSP 1
#include "../../include/RTcmix_API.h"

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

using namespace c74::min;

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

class rtcmix_tilde : public object<rtcmix_tilde>, public sample_operator<1, 1> {
public:
    MIN_DESCRIPTION {"RTcmix integration for MaxMSP"};
    MIN_TAGS {"audio, synthesis"};
    MIN_AUTHOR {"Brad Garton, ported to MinAPI by Doug Scott"};
    MIN_RELATED {"rtcmix~, rtcmix, audio"};

    // Attributes
    attribute<int> num_inputs { this, "num_inputs", MSP_INPUTS };
    attribute<int> num_outputs { this, "num_outputs", MSP_OUTPUTS };

    // Outlets
    outlet<> bang_out { this, "bang" };

    // Constructor
    rtcmix_tilde(const atoms& args = {}) {
        srate = 44100;
        num_inputs = MSP_INPUTS;
        num_outputs = MSP_OUTPUTS;

        rtcmix_load_dylib();  // Load RTcmix library
        initialize_callbacks();
    }

    // Destructor
    ~rtcmix_tilde() {
        rtcmix_destroy();
    }

    // Perform DSP: This is where the audio computation happens
    void operator()(audio_bundle input, audio_bundle output) {
        size_t frames = input.frame_count();
        std::vector<float> in_buffer(frames * num_inputs);
        std::vector<float> out_buffer(frames * num_outputs);

        for (size_t i = 0; i < frames; ++i) {
            for (int j = 0; j < num_inputs; ++j) {
                in_buffer[i * num_inputs + j] = input.samples(j)[i];
            }
        }

        rtcmix_run_audio(in_buffer.data(), out_buffer.data(), frames);

        for (size_t i = 0; i < frames; ++i) {
            for (int j = 0; j < num_outputs; ++j) {
                output.samples(j)[i] = out_buffer[i * num_outputs + j];
            }
        }
    }

    // Handle float messages (e.g., setting PField values)
    message<> float_input { this, "float", "Set PField values",
                            MIN_FUNCTION {
                                    int inlet_num = inlet;
                                    float value = args[0];
                                    if (inlet_num < num_inputs) {
                                        // Handle normal input
                                        in[inlet_num] = value;
                                    } else {
                                        // Handle PField input
                                        pfield_set(inlet_num - num_inputs + 1, value);
                                    }
                                    return {};
                            }
    };

    // Handle bang messages
    message<> bang_message { this, "bang", "Bang the current script.",
                             MIN_FUNCTION {
                                     rtcmix_run_current_script();
                                     bang_out.send();
                                     return {};
                             }
    };

    // Handle script text messages
    message<> text_message { this, "text", "Execute RTcmix script text.",
                             MIN_FUNCTION {
                                     std::string script;
                                     for (auto& arg : args) {
                                         script += arg;
                                         script += " ";
                                     }
                                     rtcmix_parse_score(script.c_str(), script.length());
                                     return {};
                             }
    };

    message<> dowrite_message{this, "savescript", "Save current script to a file.",
                              MIN_FUNCTION{
                                      // Ensure there's a script to save
                                      if (current_script.empty()) {
                                          error("No script to save.");
                                          return {};
                                      }

                                      // Choose filename
                                      std::string filename = "script.txt"; // Default filename
                                      if (!script_names[current_script_index].empty()) {
                                          filename = script_names[current_script_index];
                                      }

                                      // If no filename specified, open save dialog
                                      if (args.size() > 0 && args[0].a_type() == atom::type::symbol) {
                                          filename = args[0];
                                      } else {
                                          // Simulate a file dialog for saving (not directly available in MinAPI)
                                          filename = open_save_dialog("Save Script As", "txt");
                                          if (filename.empty()) {
                                              post("User canceled the save dialog.");
                                              return {};
                                          }
                                      }

                                      // Open file for writing
                                      std::ofstream file(filename);
                                      if (!file) {
                                          error("Failed to open file for writing: " + filename);
                                          return {};
                                      }

                                      // Write the current script to the file
                                      file << current_script;
                                      if (file.fail()) {
                                          error("Error writing to file: " + filename);
                                          return {};
                                      }

                                      file.close();
                                      post("Script saved successfully to: " + filename);

                                      return {};
                              }
    };

private:
    // RTcmix API function pointers
    rtcmixinitFunctionPtr rtcmix_init;
    rtcmixdestroyFunctionPtr rtcmix_destroy;
    rtcmixrunAudioFunctionPtr rtcmix_run_audio;
    rtsetparamsFunctionPtr rtcmix_set_params;
    parse_scoreFunctionPtr rtcmix_parse_score;
    pfield_setFunctionPtr pfield_set;

    // Variables for inputs, outputs, etc.
    float srate;
    std::vector<float> in, out;

    // Load RTcmix dylib and initialize function pointers
    void rtcmix_load_dylib() {
        rtcmix_init = load_rtcmix_function<rtcmixinitFunctionPtr>("RTcmix_init");
        rtcmix_destroy = load_rtcmix_function<rtcmixdestroyFunctionPtr>("RTcmix_destroy");
        rtcmix_run_audio = load_rtcmix_function<rtcmixrunAudioFunctionPtr>("RTcmix_runAudio");
        rtcmix_set_params = load_rtcmix_function<rtsetparamsFunctionPtr>("RTcmix_setparams");
        rtcmix_parse_score = load_rtcmix_function<parse_scoreFunctionPtr>("RTcmix_parseScore");
        pfield_set = load_rtcmix_function<pfield_setFunctionPtr>("RTcmix_setPField");

        if (rtcmix_init) {
            rtcmix_init();
        } else {
            error("Failed to load RTcmix functions");
        }
    }

    // Initialize the RTcmix callbacks
    void initialize_callbacks() {
        setBangCallback(rtcmix_bang_callback, this);
        setValuesCallback(rtcmix_values_callback, this);
    }

    // Run the current RTcmix script
    void rtcmix_run_current_script() {
        // Implement RTcmix script running logic
    }

    // Utility to load function pointers from the RTcmix dylib
    template<typename T>
    T load_rtcmix_function(const std::string& function_name) {
        T func_ptr = (T)dlsym(dlopen("rtcmixdylib.so", RTLD_LAZY), function_name.c_str());
        if (!func_ptr) {
            error("Could not load function: " + function_name);
        }
        return func_ptr;
    }

    // RTcmix callbacks
    static void rtcmix_bang_callback(void* context) {
        rtcmix_tilde* x = static_cast<rtcmix_tilde*>(context);
        x->bang_out.send();
    }

    static void rtcmix_values_callback(float* values, int num_values, void* context) {
        rtcmix_tilde* x = static_cast<rtcmix_tilde*>(context);
        // Handle RTcmix value outputs here
    }
};

MIN_EXTERNAL(rtcmix_tilde);
