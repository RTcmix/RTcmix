


#include <iostream>
#include <cstring>

#include "RTOSCListener.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"
#include "rtcmix_parse.h"

//int parse_score_buffer(const char *buffer, int buflen);


RTOSCListener::RTOSCListener(int (*parseCallback)(const char*, int)){
    this->parseCallback = parseCallback;
}


void RTOSCListener::ProcessMessage(const osc::ReceivedMessage& m,
                              const IpEndpointName& remoteEndpoint){

    (void) remoteEndpoint; // suppress unused parameter warning

    int status;
    if(std::strcmp(m.AddressPattern(), "/RTcmix/ScoreCommands") == 0){    
        osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
        const char *rtScript;
        args >> rtScript >> osc::EndMessage;

        std::cout << "<<< OSC Server Recieved Script >>>" << std::endl;
        std::cout << rtScript << std::endl;

        status = (*parseCallback)(rtScript, std::strlen(rtScript));
    }
}




