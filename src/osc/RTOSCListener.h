#include "lo/lo.h"

int score_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data);

lo_server_thread* start_osc_thread(int (*parseCallback)(const char*, int));

