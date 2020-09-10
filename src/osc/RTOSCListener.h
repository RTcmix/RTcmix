

#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"


class RTOSCListener : public osc::OscPacketListener {

    public:
        RTOSCListener(int (*parseCallback)(const char*, int));

    protected:

        int (*parseCallback)(const char*, int);  

        virtual void ProcessMessage( const osc::ReceivedMessage& m,
                                 const IpEndpointName& remoteEndpoint);


};



/*
extern "C" RTOSCListener* create_object();

extern "C" void destroy_object( RTOSCListener* object );
*/

