#include "lo/lo.h"

#define DEFAULT_OSC_PORT "7777"

lo_server_thread start_osc_thread(const char *osc_port, int (*parseCallback)(const char*, int));

