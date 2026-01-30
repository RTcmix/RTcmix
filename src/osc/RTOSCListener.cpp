#include <iostream>
#include <cstring>

#include "RTOSCListener.h"
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
        std::cout << "Received score: \n\n" << theScore << std::endl;
        int parseStatus = (*parseCallback)(theScore, std::strlen(theScore));
    } else {
        std::cerr << "Received unknown typespec: " << types << std::endl;
    }
    return 0;
}

void osc_err_handler(int num, const char *msg, const char *where) {
    std::cerr << "OSC server failure, code " << num << ": " << (msg ? msg : "(no detail)") << std::endl;
    exit(1);
}

lo_server_thread start_osc_thread(const char *osc_port, int (*parseCallback)(const char*, int))
{
    lo_server_thread st = lo_server_thread_new(osc_port, osc_err_handler);
    if (st != NULL) {
        lo_server_thread_add_method(st, "/RTcmix/ScoreCommands", "s",
                &score_handler, (void*) parseCallback);
        if (lo_server_thread_add_method(st, "/RTcmix/*", NULL,
                &RTcmixMain::command_handler, (void*) NULL) == NULL) {
            std::cerr << "ERROR: installed version of the OSC library must be >= 0.32" << std::endl;
            exit(1);
        }
        if (lo_server_thread_add_method(st, NULL, NULL,
                &RTcmixMain::default_osc_handler, (void*) NULL) == NULL) {
            std::cerr << "ERROR: installed version of the OSC library must be >= 0.32" << std::endl;
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
