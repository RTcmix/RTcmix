#include <iostream>
#include <cstring>

#include "RTOSCListener.h"
#include "lo/lo.h"

int score_handler(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){

    int (*parseCallback)(const char*, int) = (int (*)(const char*, int)) user_data;

    char *theScore = &argv[0]->s;
    std::cout << "Received score: \n\n" << theScore << std::endl;
    int parseStatus;
    parseStatus = (*parseCallback)(theScore, std::strlen(theScore));
    return 0;
}

// BGGx
int modular_handler(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data) {

	char *theSpec = &argv[0]->s;
	printf("theSpec: %s\n", theSpec);

	return 0;
}

lo_server_thread* start_osc_thread(int (*parseCallback)(const char*, int)){
    lo_server_thread *st = NULL;
    st = (lo_server_thread*) malloc(sizeof(lo_server_thread));
    *st = lo_server_thread_new("7777", NULL);

    lo_server_thread_add_method(*st, "/RTcmix/ScoreCommands", "s",
            &score_handler, (void*) parseCallback);

// BGGx
    lo_server_thread_add_method(*st, "/modular", "s",
            &modular_handler, (void*) parseCallback);
    
    lo_server_thread_start(*st);

    return st;
}


