#include <iostream>
#include <cstring>

#include "RTOSCListener.h"
#include "lo/lo.h"

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
    std::cerr << "OSC server startup failure, code " << num << ": " << (msg ? msg : "(no detail)") << std::endl;
    exit(1);
}

lo_server_thread start_osc_thread(const char *osc_port, int (*parseCallback)(const char*, int))
{
    lo_server_thread st = lo_server_thread_new(osc_port, osc_err_handler);

    if (st != NULL) {
        lo_server_thread_add_method(st, "/RTcmix/ScoreCommands", "s",
                &score_handler, (void*) parseCallback);

        lo_server_thread_start(st);
    }
    return st;
}


