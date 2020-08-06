#include "osc_funcs.h"
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <unistd.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 

#include <unordered_map>

#include <lo/lo.h>
#include "RTcmix_API.h"

struct clnt_addr{
    std::string *port;
    std::string *ip;


    clnt_addr(char *port, char *ip){
        this->port = new std::string(port);
        this->ip = new std::string(ip);
    }

};


void liblo_error(int num, const char *msg, const char *path){
    std::cout << "liblo server error " << num << " in path " << path 
              << ": " << msg << "\n" << std::endl;
    fflush(stdout);
}

int rtcmix_instr_hnldr(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data){
    char* command = &argv[0]->S;
    int status;
    std::cout << command << std::endl;
    status = RTcmix_parseScore(command, strlen(command));
    std::cout << "OSC ParseScore STATUS: " << status << std::endl;
    fflush(stdout);
    return 0;
}


void RTcmix_setPField(int inlet, float pval);
int rtcmix_pfield_hnldr(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data, void *user_data){
    int inlet = argv[0]->i;
    float pval = argv[1]->f;
    std::cout << "Got Pfields: inlet-" << inlet << " pval-" << pval << std::endl;
    RTcmix_setPField(inlet, pval);
    fflush(stdout);
    return 0;
}




int rtcmix_get_client_addrs(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){

    char *clnt_ip = &argv[0]->S;
    char *clnt_port = &argv[1]->S;
    char *clnt_name = &argv[2]->S;

    update_client_namemap(clnt_ip, clnt_port, clnt_name);

    //clnt_addr *c = new clnt_addr(clnt_port, clnt_ip);
    //(*client_addrs)[std::string(clnt_name)] = c;
    
    fflush(stdout);
    return 0;

}


void update_client_namemap(char *clnt_ip, char *clnt_port, char *clnt_name){

    clnt_addr *c = new clnt_addr(clnt_port, clnt_ip);
    (*client_addrs)[std::string(clnt_name)] = c;

}



int sendOSC_str(char *clnt_name, char *osc_path, char *message){

    clnt_addr *c = (*client_addrs)[std::string(clnt_name)];
    std::string *clnt_ip = c->ip;
    std::string *clnt_port = c->port;

    lo_address t = lo_address_new(clnt_ip->c_str(), clnt_port->c_str());
   
    if(lo_send(t, osc_path, "S", message)==-1){
        printf("OSC ERROR %d: %s\n", lo_address_errno(t), lo_address_errstr(t));
        return 1;
    }
    return 0;
}

int sendOSC_int(char *clnt_name, char *osc_path, int message){
  
    clnt_addr *c = (*client_addrs)[std::string(clnt_name)];
    std::string *clnt_ip = c->ip;
    std::string *clnt_port = c->port;

    lo_address t = lo_address_new(clnt_ip->c_str(), clnt_port->c_str());
   
    if(lo_send(t, osc_path, "i", message)==-1){
        printf("OSC ERROR %d: %s\n", lo_address_errno(t), lo_address_errstr(t));
        return 1;
    }
    return 0;
}

int sendOSC_flt(char *clnt_name, char *osc_path, float message){
  
    clnt_addr *c = (*client_addrs)[std::string(clnt_name)];
    std::string *clnt_ip = c->ip;
    std::string *clnt_port = c->port;

    lo_address t = lo_address_new(clnt_ip->c_str(), clnt_port->c_str());
   
    if(lo_send(t, osc_path, "f", message)==-1){
        printf("OSC ERROR %d: %s\n", lo_address_errno(t), lo_address_errstr(t));
        return 1;
    }
    return 0;
}




int sendOSC_anyformat(char *clnt_name, char *osc_path, char *format, ...){
  
    //TOTALLY DOESN'T WORK!

    std::cout << "in send" << std::endl;
    clnt_addr *c = (*client_addrs)[std::string(clnt_name)];
    std::cout << "test3" << std::endl;
    //std::string *clnt_ip = c->ip;
    std::cout << "test4" << std::endl;
    //std::string *clnt_port = c->port;
    std::cout << "test5" << std::endl;

   // int clnt_port = clnt_addr_store.sin_port;
   // char *clnt_ip = NULL;
   // inet_ntop(AF_INET, &(clnt_addr_store.sin_addr), clnt_ip, INET_ADDRSTRLEN);
   // const char *port_str = std::to_string(clnt_port).c_str();
  

    //lo_address t = lo_address_new(clnt_ip->c_str(), clnt_port->c_str());
    lo_address t = lo_address_new("localhost", "6543");
    std::cout << "test6" << std::endl;

    va_list args;
    std::cout << "test7" << std::endl;
    va_start(args, format);
    std::cout << "test8" << std::endl;
    std::cout << format << std::endl;
    double val = va_arg(args, double);
    std::cout << val << std::endl;
    
    if(lo_send(t, osc_path, format, args)==-1){
        printf("OSC ERROR %d: %s\n", lo_address_errno(t), lo_address_errstr(t));
        return 1;
    }
    va_end(args);
    return 0;
}



/*I JUST USED THIS TO TEST RTCMIX SENDING OSC, IT'S REALLY USELESS OTHERWISE*/
int rtcmix_test_send_str(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){
    char *name = &argv[0]->S;
    if(sendOSC_str(name, "/test_receive_str", "some string from rtcmix")!=0){
        printf("error sending");
    }

    fflush(stdout);
    return 0;
}

int rtcmix_test_send_int(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){
    char *name = &argv[0]->S;
    if(sendOSC_int(name, "/test_receive_int", 11010101)!=0){
        printf("error sending");
    }

    fflush(stdout);
    return 0;
}

int rtcmix_test_send_flt(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){
    char *name = &argv[0]->S;
    if(sendOSC_flt(name, "/test_receive_flt", 3.141592)!=0){
        printf("error sending");
    }

    fflush(stdout);
    return 0;
}

int rtcmix_test_send_any(const char *path, const char *types, lo_arg ** argv,
        int argc, void *data, void *user_data){
    char *name = &argv[0]->S;
    if(sendOSC_anyformat(name, "/test_receive_any", "d", 3.14)!=0){
        printf("error sending");
    }

    fflush(stdout);
    return 0;
}



int quit_handler(const char *path, const char *types, lo_arg ** argv,
                 int argc, void *data, void *user_data){
    quit_osc_signal = 1;
    printf("quiting\n\n");
    fflush(stdout);
    return 0;
}



void* rtcmix_osc_thread(void *data){

    client_addrs = new std::unordered_map<std::string, clnt_addr*>();

    /* start a new server on port 7777 */
    st = lo_server_thread_new("7777", //NULL);
           liblo_error);

    /* add method matching path /rtc_inst, with an OSC-string*/
    lo_server_thread_add_method(st, "/rtc_inst", "S",
            &rtcmix_instr_hnldr, NULL);
   
    /*add method matching path /rtcmix/pfield with an int and float*/
    lo_server_thread_add_method(st, "/rtcmix/pfield", "if", 
            &rtcmix_pfield_hnldr, NULL);
    
    /*add method matching path /rtcmix/store_addrs with SSS*/
    lo_server_thread_add_method(st, "/rtcmix/store_addrs", "SSS",
            &rtcmix_get_client_addrs, NULL);


    lo_server_thread_add_method(st, "/rtcmix/test_send_str", "S",
            &rtcmix_test_send_str, NULL);

    lo_server_thread_add_method(st, "/rtcmix/test_send_int", "S",
            &rtcmix_test_send_int, NULL);

    lo_server_thread_add_method(st, "/rtcmix/test_send_flt", "S",
            &rtcmix_test_send_flt, NULL);

    lo_server_thread_add_method(st, "/rtcmix/test_send_any", "S",
            &rtcmix_test_send_any, NULL);


    /* add method that will match the path /quit with no args */
    lo_server_thread_add_method(st, "/quit", "", 
            &quit_handler, NULL);


    lo_server_thread_start(st);
    quit_osc_signal = 0;
    while(!quit_osc_signal){
        usleep(1000);
    }
    delete client_addrs;
    lo_server_thread_free(st);

    return NULL;
}




