#include "lo/lo.h"

#define DEFAULT_OSC_PORT "7777"

const char *get_osc_init_script_path();

lo_server_thread start_osc_thread(const char *osc_port, int (*parseCallback)(const char*, int));

