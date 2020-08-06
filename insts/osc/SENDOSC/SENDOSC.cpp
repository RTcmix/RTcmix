/* SENDOSC -- send a scheduled OSC message

	p0 = time to send the message
	p1 = OSC tag
	p2 = OSC data

	BGG/Michel 7/2020

        p1 = client name
        p2 = OSC path
        p3 = format
        p4 = value
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "SENDOSC.h"
#include <rt.h>
#include <rtdefs.h>
#include <PField.h>

#include <osc_funcs.h>
#include <iostream>

SENDOSC::SENDOSC() : Instrument()
{
}

SENDOSC::~SENDOSC()
{
}


//DECLARE MY SENDOSC FUNCTIONS HERE FROM OSC_FUNCS
//extern int sendOSC(char *oscmessage, float *oscvals);
int sendOSC_str(char *clnt_name, char *osc_path, char *message);
int sendOSC_int(char *clnt_name, char *osc_path, int message);
int sendOSC_flt(char *clnt_name, char *osc_path, float message);


int SENDOSC::init(double p[], int n_args)
{
	if (rtsetoutput(p[0], 0, this) == -1){
		return DONT_SCHEDULE;
        }

        const PField &field1 = getPField(1);
        sprintf(clnt_name, "%s", field1.stringValue(0));

        //update_client_namemap("localhost", "6544", "michel"); //for testing

        /*if (client_addrs->find(clnt_name) == client_addrs->end()){
            //Name not present, need an error
            update_client_namemap("localhost", "6543", "michel");
        }*/

        const PField &field2 = getPField(2);
        sprintf(osc_path, "%s", field2.stringValue(0));

        const PField &field3 = getPField(3);
        sprintf(format, "%s", field3.stringValue(0));

        const PField &field4 = getPField(4);
        if(format[0] == 'i'){
            int_message = field4.intValue(0);
        } else if (format[0] == 'f'){
            flt_message = (float) field4.doubleValue(0);
        } else if (format[0] == 'S'){
            sprintf(str_message, "%s", field4.stringValue(0));
        }/* else {
           bad format, need to have an error here or something
        }*/

        //update_client_namemap("localhost", "6543", "michel");

        //for (int i = 0; i < n_args; i++){
	//	oscvals[i] = p[2+i];
        //        std::cout << "oscval! " << oscvals[i] << std::endl;
        //}

	return nSamps();
}

int SENDOSC::run()
{
        if(format[0] == 'i'){
            sendOSC_int(clnt_name, osc_path, int_message);
        } else if (format[0] == 'f'){
            sendOSC_flt(clnt_name, osc_path, flt_message);
        } else if (format[0] == 'S'){
            sendOSC_str(clnt_name, osc_path, str_message);
        }
	
        return(1);
}


Instrument*
makeSENDOSC()
{
	SENDOSC *inst;

	inst = new SENDOSC();
	inst->set_bus_config("SENDOSC");

	return inst;
}

#ifndef EMBEDDED
void
rtprofile()
{
	RT_INTRO("SENDOSC",makeSENDOSC);
}
#endif
