#ifndef _OSCFUNCS_H_
#define _OSCFUNCS_H_

#include <lo/lo.h>
#include <unordered_map>

lo_server_thread st;
int quit_osc_signal;
struct clnt_addr;
std::unordered_map<std::string, clnt_addr*> *client_addrs;

void *rtcmix_osc_thread(void *data);
void liblo_error(int num, const char *msg, const char *path);
int rtcmix_instr_hnldr(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data);
int rtcmix_pfield_hndlr(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data);
int rtcmix_get_client_addrs(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data);
void update_client_namemap(char *clnt_ip, char *clnt_port, char *clnt_name);

int sendOSC_str(char *clnt_name, char *osc_path, char *message);
int sendOSC_int(char *clnt_name, char *osc_path, char *message);
int sendOSC_flt(char *clnt_name, char *osc_path, char *message);

int quit_handler(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data);

        

#endif	// _OSCFUNCS_H_




