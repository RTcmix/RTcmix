#include <cstring>
#include <sys/stat.h>

#include "RTOSCListener.h"

#include "RTOption.h"
#include <ugens.h>
#include "../rtcmix/RTcmixMain.h"
#include "lo/lo.h"

// This is for future debugging
static void print_version() {
    char verstr[64];
    char extra[64];
    int major, minor;
    int lt_major, lt_minor, lt_bug;

    // Initialize strings
    memset(verstr, 0, sizeof(verstr));
    memset(extra, 0, sizeof(extra));

    lo_version(verstr, sizeof(verstr),
               &major, &minor,
               extra, sizeof(extra),
               &lt_major, &lt_minor, &lt_bug);

    printf("liblo version string: %s\n", verstr);
    printf("liblo version:        %d.%d%s\n",
           major, minor, extra);
    printf("libtool version:      %d.%d.%d\n",
           lt_major, lt_minor, lt_bug);
}

int score_handler(const char *path, const char *types, lo_arg **argv,
                  int argc, lo_message data, void *user_data)
{
    int (*parseCallback)(const char *, int) = (int (*)(const char *, int)) user_data;
    if (std::strncmp(types, "s", 1) == 0) {
        char *theScore = &argv[0]->s;
        rtcmix_advise("score_handler", "Received score: \n\n%s", theScore);
        int parseStatus = (*parseCallback)(theScore, std::strlen(theScore));
    } else {
        rterror("score_handler", "Received unknown typespec: '%s'", types);
    }
    return 0;
}

int scorefile_handler(const char *path, const char *types, lo_arg **argv,
                  int argc, lo_message data, void *user_data)
{
    int (*parseCallback)(const char *, int) = (int (*)(const char *, int)) user_data;
    if (std::strncmp(types, "s", 1) == 0) {
        const char *pathToScore = &argv[0]->s;
        rtcmix_advise("scorefile_handler", "Received path to score: '%s'", pathToScore);
        char *scoreBuffer = RTcmixMain::readScoreFile(pathToScore);
        if (scoreBuffer != NULL) {
            int parseStatus = (*parseCallback)(scoreBuffer, std::strlen(scoreBuffer));
            rtcmix_advise("scorefile_handler", "Finished parsing score");
            delete [] scoreBuffer;
        }
   } else {
        rterror("scorefile_handler", "Received unknown typespec: '%s'", types);
    }
    return 0;
}

void osc_err_handler(int num, const char *msg, const char *where) {
    rterror("osc_err_handler", "OSC server failure, code %d: %s", num, msg ? msg : "(no detail)");
    exit(1);
}

lo_server_thread start_osc_thread(const char *osc_port, int (*parseCallback)(const char*, int))
{
    lo_server_thread st = lo_server_thread_new(osc_port, osc_err_handler);
    if (st != NULL) {
        // Score handed in as one text buffer
        lo_server_thread_add_method(st, "/RTcmix/ScoreCommands", "s",
                &score_handler, (void*) parseCallback);
        // File path to score handed in
        lo_server_thread_add_method(st, "/RTcmix/ScoreFile", "s",
                &scorefile_handler, (void*) parseCallback);
        // Anything else
        if (lo_server_thread_add_method(st, "/RTcmix/*", NULL,
                &RTcmixMain::command_handler, (void*) NULL) == NULL) {
            rterror("start_osc_thread", "Installed version of the OSC library must be >= 0.32");
            exit(1);
        }
        if (lo_server_thread_add_method(st, NULL, NULL,
                &RTcmixMain::default_osc_handler, (void*) NULL) == NULL) {
            rterror("start_osc_thread", "Installed version of the OSC library must be >= 0.32");
            exit(1);
        }
        lo_server_thread_start(st);
    }
    return st;
}

#ifndef SCRIPT_PATH
#error Build system failed to provide macro for SCRIPT_PATH
#endif

const char *get_osc_init_script_path()
{
    static char script_path[256];
    snprintf(script_path, 256, SCRIPT_PATH);
    return script_path;
}
