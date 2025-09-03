/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OSXDISPLAY_H_
#define _OSXDISPLAY_H_
#include <RTcmixDisplay.h>
#include <Carbon/Carbon.h>
#include "display_ipc.h"

class OSXDisplay : public RTcmixDisplay {
public:
	OSXDisplay();
	virtual ~OSXDisplay();
	virtual int show();

protected:
	// RTcmixDisplay reimplementations
	virtual void doConfigureLabel(int id, const char *prefix,
                                 const char *units, int precision);
	virtual void doUpdateLabelValue(int id, double value);
	virtual bool handleEvents();

private:
	int openSocket();
	int reportError(const char *err, bool useErrno);
	int readPacket(DisplaySockPacket *packet);
	int writePacket(const DisplaySockPacket *packet);
	void sendLabel(int id, const char *prefix,
                  const char *units, int precision);
	void sendLabelValue(int id, double value);
	int pollInput(long);

	int _sockport;
	int _sockdesc;
	DisplaySockPacket *_packet;
	DisplaySockPacket *_evtpacket;
	char *_servername;
	bool _running;
};

#endif // _OSXDISPlAY_H_
