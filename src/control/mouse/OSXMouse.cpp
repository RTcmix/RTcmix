/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <OSXMouse.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <float.h>

//#define DEBUG


OSXMouse::OSXMouse() : RTcmixMouse()
{
	_x = -DBL_MAX;		// forces getpos* to return negative
	_y = DBL_MAX;
	_sockport = SOCK_PORT;
	_packet = new MouseSockPacket [1];
}

OSXMouse::~OSXMouse()
{
// XXX send quit msg to MouseWindow?
	delete [] _packet;
}

int OSXMouse::show()
{
// XXX launch MouseWindow.app if it's not already running, establish socket
// connection as client, handshake
	return 0;
}

int OSXMouse::reportError(const char *err)
{
	fprintf(stderr, "%s\n", err);
	return -1;
}

int OSXMouse::readPacket(MouseSockPacket *packet)
{
	char *ptr = (char *) packet;
	const int packetsize = sizeof(MouseSockPacket);
	ssize_t amt = 0;
	do {
		ssize_t n = read(_newdesc, ptr + amt, packetsize - amt);
		if (n < 0)
			return reportError(strerror(errno));
		amt += n;
	} while (amt < packetsize);

	return 0;
}

int OSXMouse::writePacket(const MouseSockPacket *packet)
{
	const char *ptr = (char *) packet;
	const int packetsize = sizeof(MouseSockPacket);
	ssize_t amt = 0;
	do {
		ssize_t n = write(_newdesc, ptr + amt, packetsize - amt);
		if (n < 0)
			return reportError(strerror(errno));
		amt += n;
	} while (amt < packetsize);

	return 0;
}

// Ensures null termination.  <len> includes the null.
void mystrncpy(char *dest, const char *src, int len)
{
	strncpy(dest, src, len - 1);
	dest[len - 1] = 0;
}

// Send prefix, units and precision to MouseWindow for given label id and axis.
void OSXMouse::sendLabel(const bool isXAxis, const int id, const char *prefix,
		const char *units, const int precision)
{
	_packet->id = id;

	_packet->type = isXAxis ? kPacketConfigureXLabelPrefix
									: kPacketConfigureYLabelPrefix;
	mystrncpy(_packet->data.str, prefix, PART_LABEL_LENGTH);
	writePacket(_packet);

	if (units) {		// units string is optional
		_packet->type = isXAxis ? kPacketConfigureXLabelUnits
										: kPacketConfigureYLabelUnits;
		mystrncpy(_packet->data.str, units, PART_LABEL_LENGTH);
		writePacket(_packet);
	}

	_packet->type = isXAxis ? kPacketConfigureXLabelPrecision
									: kPacketConfigureXLabelPrecision;
	_packet->data.precision = precision;
	writePacket(_packet);
}

void OSXMouse::doConfigureXLabel(const int id, const char *prefix,
		const char *units, const int precision)
{
	sendLabel(true, id, prefix, units, precision);
}

void OSXMouse::doConfigureYLabel(const int id, const char *prefix,
		const char *units, const int precision)
{
	sendLabel(false, id, prefix, units, precision);
}

// Send label value to MouseWindow for given label id and axis.
void OSXMouse::sendLabelValue(const bool isXAxis, const int id,
	const double value)
{
	_packet->id = id;
	_packet->type = isXAxis ? kPacketUpdateXLabel : kPacketUpdateYLabel;
	_packet->data.value = value;
	writePacket(_packet);
}

void OSXMouse::doUpdateXLabelValue(const int id, const double value)
{
	sendLabelValue(true, id, value);
}

void OSXMouse::doUpdateYLabelValue(const int id, const double value)
{
	sendLabelValue(false, id, value);
}

bool OSXMouse::handleEvents()
{
// XXX read incoming data from MouseWindow.app
// throw away all but the last set of x/y coords.
// Question: should we ever exit loop when data is still available to read?
// Yes, we must exit every time we read a message, because otherwise
// we'd never be able to receive the configure label message!
//
// Each message will have a message type: coords (in range [0,1]),
// quit (meaning that user has quit MouseWindow app before we're done),
// and ack (which is handled in ctor and dtor instead of here).

	return true;
}

