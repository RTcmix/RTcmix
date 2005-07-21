/* RTcmix - Copyright (C) 2005  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// A simple Carbon application that receives setup information from RTcmix
// over a socket, sends back mouse coordinates in range [0,1], and receives
// scaled data values for label display.  It's supposed to support the same
// functionality as the version for X.
//
// The reason for writing this separate program is that a command-line
// program (such as CMIX) can't put up a GUI window in OSX and still have
// events work correctly.
//
// -John Gibson, 1/05/05

#include <Carbon/Carbon.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include "labels.h"
#include "mouse_ipc.h"

//#define DEBUG

// Important tuning parameter: how often to nap between polling for 
// incoming packets.
static const int kSleepMsec = 10;

static const char *kLabelFontName = "Monoco";
static const int kLabelFontSize = 10;
static const int kLabelFontFace = 0;		// i.e., plain

static const int _titleBarHeight = 22;	// FIXME: should get this from system
static const int _labelXpos = kLabelFromLeft;
static const int _labelYpos = kLabelFromTop;
static const int _maxLabelChars = kWholeLabelLength;

static int _xlabelCount;
static int _ylabelCount;
static char *_xlabel[kNumLabels];
static char *_ylabel[kNumLabels];
static char *_xprefix[kNumLabels];
static char *_yprefix[kNumLabels];
static char *_xunits[kNumLabels];
static char *_yunits[kNumLabels];
static int _xprecision[kNumLabels];
static int _yprecision[kNumLabels];

static double _xfactor;
static double _yfactor;

static int _lineHeight = 0;
static int _charWidth = 0;
static int _fontAscent = 0;
static WindowRef _window;

// Default window position and size
enum {
	kWindowXpos = 100,
	kWindowYpos = 100,
	kWindowWidth = 300,
	kWindowHeight = 300
};

// socket
static int _servdesc;
static int _newdesc;
static int _sockport = kSockPort;

// thread
static bool _runThread;
static pthread_t _listenerThread;

void drawXLabels();
void drawYLabels();
int closeSocket();


// =============================================================================
// Utilities

int reportConsoleError(const char *err, const bool useErrno)
{
	if (useErrno)
		fprintf(stderr, "%s: %s\n", err, strerror(errno));
	else
		fprintf(stderr, "%s\n", err);
	return -1;
}

int reportError(const char *err, const bool useErrno)
{
//FIXME: pop alert instead of console print?  maybe not
	reportConsoleError(err, useErrno);
	return -1;
}


// =============================================================================
// IPC stuff

// Read one packet, and store in <packet>.  Return 0 if okay, -1 if error,
// and 1 if EOF.
int readPacket(MouseSockPacket *packet)
{
	char *ptr = (char *) packet;
	const int packetsize = sizeof(MouseSockPacket);
	ssize_t amt = 0;
	do {
		ssize_t n = read(_newdesc, ptr + amt, packetsize - amt);
		if (n == -1)
			return reportError("readPacket", true);
		else if (n == 0)	// EOF
			return 1;
		amt += n;
	} while (amt < packetsize);

	return 0;
}

int writePacket(const MouseSockPacket *packet)
{
	const char *ptr = (char *) packet;
	const int packetsize = sizeof(MouseSockPacket);
	ssize_t amt = 0;
	do {
		ssize_t n = write(_newdesc, ptr + amt, packetsize - amt);
		if (n < 0)
			return reportError("writePacket", true);
		amt += n;
	} while (amt < packetsize);

	return 0;
}

// Set prefix string for xlabel with <id>, allocate a new xlabel, and increment
// the count of xlabels in use.
void configureXLabelPrefix(const int id, const char *prefix)
{
	assert(id >= 0 && id < kNumLabels);
	_xprefix[id] = strdup(prefix);
	_xlabel[id] = new char [kWholeLabelLength];
	_xlabel[id][0] = 0;
	_xlabelCount++;
}

// Set (optional) units string for xlabel with <id>.
// NOTE: This will have no effect if we don't receive a prefix for this label.
void configureXLabelUnits(const int id, const char *units)
{
	assert(id >= 0 && id < kNumLabels);
	_xunits[id] = strdup(units);
}

// Set precision for xlabel with <id>.
// NOTE: This will have no effect if we don't receive a prefix for this label.
void configureXLabelPrecision(const int id, const int precision)
{
	assert(id >= 0 && id < kNumLabels);
	_xprecision[id] = precision;
}

// Set prefix string for ylabel with <id>, allocate a new ylabel, and increment
// the count of ylabels in use.
void configureYLabelPrefix(const int id, const char *prefix)
{
	assert(id >= 0 && id < kNumLabels);
	_yprefix[id] = strdup(prefix);
	_ylabel[id] = new char [kWholeLabelLength];
	_ylabel[id][0] = 0;
	_ylabelCount++;
}

// Set (optional) units string for ylabel with <id>.
// NOTE: This will have no effect if we don't receive a prefix for this label.
void configureYLabelUnits(const int id, const char *units)
{
	assert(id >= 0 && id < kNumLabels);
	_yunits[id] = strdup(units);
}

// Set precision for ylabel with <id>.
// NOTE: This will have no effect if we don't receive a prefix for this label.
void configureYLabelPrecision(const int id, const int precision)
{
	assert(id >= 0 && id < kNumLabels);
	_yprecision[id] = precision;
}

void updateXLabelValue(const int id, const double value)
{
	assert(id >= 0 && id < kNumLabels);
	const char *units = _xunits[id] ? _xunits[id] : "";
	snprintf(_xlabel[id], kWholeLabelLength, "%s: %.*f %s",
				_xprefix[id], _xprecision[id], value, units);
	drawXLabels();
}

void updateYLabelValue(const int id, const double value)
{
	assert(id >= 0 && id < kNumLabels);
	const char *units = _yunits[id] ? _yunits[id] : "";
	snprintf(_ylabel[id], kWholeLabelLength, "%s: %.*f %s",
				_yprefix[id], _yprecision[id], value, units);
	drawYLabels();
}

// Send coordinates in range [0,1] back to RTcmix.
void sendCoordinates(const int x, const int y)
{
	const double xscaled = x * _xfactor;
	const double yscaled = 1.0 - (y * _yfactor);

	static MouseSockPacket *packet = NULL;
	if (packet == NULL) {
		packet = new MouseSockPacket [1];
		packet->type = kPacketMouseCoords;
	}

	packet->data.point.x = xscaled;
	packet->data.point.y = yscaled;

	writePacket(packet);
}


// =============================================================================
// Event callbacks and friends

pascal OSStatus doAppMouseMoved(EventHandlerCallRef nextHandler,
	EventRef theEvent, void *userData)
{
	SetThemeCursor(kThemeArrowCursor);
	return noErr;
}

void drawXLabels()
{
	if (_xlabelCount <= 0)
		return;

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(GetWindowPort(_window));

	// Clear rect enclosing all X labels.
	const int height = _xlabelCount * _lineHeight;
	const int width = _maxLabelChars * _charWidth;
	int ypos = _labelYpos;
	Rect rect;
	SetRect(&rect, _labelXpos, ypos, _labelXpos + width, ypos + height);
	EraseRect(&rect);
#ifdef DEBUG
	FrameRect(&rect);
//	printf("drawXLabels: xpos=%d, ypos=%d, width=%d, height=%d\n",
//				_labelXpos, ypos, width, height);
#endif

	// Draw all X labels.
	ypos += _fontAscent;
	int line = 0;
	for (int i = 0; i < _xlabelCount; i++) {
		Str255 str;
		CopyCStringToPascal(_xlabel[i], str);
		MoveTo(_labelXpos, ypos + (line * _lineHeight));
		DrawString(str);
		line++;
	}

	SetPort(oldPort);
}

void drawYLabels()
{
	if (_ylabelCount <= 0)
		return;

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(GetWindowPort(_window));

	// Clear rect enclosing all Y labels.
	const int height = _ylabelCount * _lineHeight;
	const int width = _maxLabelChars * _charWidth;
	int ypos = _labelYpos + (_xlabelCount * _lineHeight);
	Rect rect;
	SetRect(&rect, _labelXpos, ypos, _labelXpos + width, ypos + height);
	EraseRect(&rect);
#ifdef DEBUG
	FrameRect(&rect);
//	printf("drawYLabels: xpos=%d, ypos=%d, width=%d, height=%d\n",
//				_labelXpos, ypos, width, height);
#endif

	// Draw all Y labels.
	ypos += _fontAscent;
	int line = 0;
	for (int i = 0; i < _ylabelCount; i++) {
		Str255 str;
		CopyCStringToPascal(_ylabel[i], str);
		MoveTo(_labelXpos, ypos + (line * _lineHeight));
		DrawString(str);
		line++;
	}

	SetPort(oldPort);
}

void drawWindowContent()
{
	drawXLabels();
	drawYLabels();
}

void setFactors()
{
	Rect rect;
	GetWindowBounds(_window, kWindowContentRgn, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	_xfactor = 1.0 / (double) (width - 1);
	_yfactor = 1.0 / (double) (height - 1);
}

// Handle events other than MouseMoved events.
pascal OSStatus doWindowEvent(EventHandlerCallRef nextHandler,
	EventRef theEvent, void *userData)
{
	OSStatus status = eventNotHandledErr;

	switch (GetEventKind(theEvent)) {
		case kEventWindowDrawContent:
			drawWindowContent();
			status = noErr;
			break;
		case kEventWindowBoundsChanged:
			setFactors();
			status = noErr;
			break;
		case kEventWindowClose:
			status = CallNextEventHandler(nextHandler, theEvent);
			// NB: window is gone now!
			if (status == noErr)
				QuitApplicationEventLoop();
			break;
		default:
			break;
	}

	return status;
}

pascal OSStatus doWindowMouseMoved(EventHandlerCallRef nextHandler,
	EventRef theEvent, void *userData)
{
	Point mouseLoc;
	GetEventParameter(theEvent, kEventParamWindowMouseLocation, typeQDPoint,
		NULL, sizeof(Point), NULL, &mouseLoc);

	const int x = mouseLoc.h;
	const int y = mouseLoc.v - _titleBarHeight;
 
//printf("x: %d, y: %d\n", x, y);

	if (y >= 0) {
		sendCoordinates(x, y);
		SetThemeCursor(kThemeCrossCursor);
	}
	else
		SetThemeCursor(kThemeArrowCursor);

	return noErr;
}


// =============================================================================
// Initialization, finalization

int createApp()
{
	const UInt32 numTypes = 1;
	EventTypeSpec eventType;

	eventType.eventClass = kEventClassMouse;
	eventType.eventKind = kEventMouseMoved;
	OSStatus status = InstallApplicationEventHandler(
					NewEventHandlerUPP(doAppMouseMoved),
					numTypes, &eventType, NULL, NULL);
	if (status != noErr)
		return reportError("createApp: Can't install app mouse event handler.",
		                                                                false);

	return 0;
}

#ifdef NOTYET
// doesn't look like we need menus, but this could be a start  -JGG
enum {
	kRootMenu = 0,
	kFileMenu = 1
};

int createMenus()
{
	MenuRef rootMenuRef = AcquireRootMenu();

	MenuAttributes attr = 0;
	MenuRef fileMenuRef;
	OSStatus status = CreateNewMenu(kFileMenu, attr, &fileMenuRef);
	if (status != noErr)
		return reportConsoleError("Can't create file menu.", false);
	SetMenuTitleWithCFString(fileMenuRef, CFSTR("File Menu"));

	InsertMenu(fileMenuRef, kInsertHierarchicalMenu);

	ShowMenuBar();

	return 0;
}
#endif

int createWindow()
{
	Rect rect;

	SetRect(&rect, kWindowYpos, kWindowXpos,
				kWindowYpos + kWindowHeight, kWindowXpos + kWindowWidth);

	OSStatus status = CreateNewWindow(kDocumentWindowClass,
								kWindowStandardDocumentAttributes,
								&rect, &_window); 
	if (status != noErr)
		return reportError("createWindow: Error creating window.", false);

	setFactors();

	SetWindowTitleWithCFString(_window, CFSTR("RTcmix Mouse Input"));

	InstallStandardEventHandler(GetWindowEventTarget(_window));

	UInt32 numTypes = 3;
	EventTypeSpec eventTypes[numTypes];

	eventTypes[0].eventClass = kEventClassWindow;
	eventTypes[0].eventKind = kEventWindowDrawContent;
	eventTypes[1].eventClass = kEventClassWindow;
	eventTypes[1].eventKind = kEventWindowBoundsChanged;
	eventTypes[2].eventClass = kEventClassWindow;
	eventTypes[2].eventKind = kEventWindowClose;
	status = InstallWindowEventHandler(_window,
					NewEventHandlerUPP(doWindowEvent),
					numTypes, eventTypes, NULL, NULL);
	if (status != noErr)
		return reportError("createWindow: Can't install close event handler.",
		                                                               false);

	numTypes = 1;
	eventTypes[0].eventClass = kEventClassMouse;
	eventTypes[0].eventKind = kEventMouseMoved;
	status = InstallWindowEventHandler(_window,
					NewEventHandlerUPP(doWindowMouseMoved),
					numTypes, eventTypes, NULL, NULL);
	if (status != noErr)
		return reportError("createWindow: Can't install mouse event handler.",
		                                                               false);

	// Get font info.
	SetPort(GetWindowPort(_window));

	// NB: This is the deprecated way, but the new way seems too complicated.
	// We'll figure it out when it's really necessary.
	Str255 str;
	CopyCStringToPascal(kLabelFontName, str);
	SInt16 fontID;
	GetFNum(str, &fontID);
	if (fontID == 0)
		fontID = applFont;
	TextFont(fontID);
	TextSize(kLabelFontSize);
	TextFace(kLabelFontFace);
	FontInfo finfo;
	GetFontInfo(&finfo);
	_charWidth = finfo.widMax;
	_lineHeight = finfo.ascent + finfo.descent;
	_fontAscent = finfo.ascent;

	return 0;
}

int pollInput(long usec)
{
	fd_set rfdset;
	FD_ZERO(&rfdset);
	FD_SET(_newdesc, &rfdset);
	const int nfds = _newdesc + 1;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = usec;
	int result = select(nfds, &rfdset, NULL, NULL, &timeout);
	if (result == -1)
		reportError("pollInput", true);
	return result;
}

// Read any incoming data from RTcmix, and dispatch messages appropriately.
void *listenerLoop(void *context)
{
	MouseSockPacket *packet = new MouseSockPacket [1];

	while (_runThread) {
		int result;
		do {
			result = pollInput(0);
			if (result == -1)
				_runThread = false;
			else if (result > 0) {
				if (readPacket(packet) != 0) {
					_runThread = false;
					break;
				}
				
				switch (packet->type) {
					case kPacketConfigureXLabelPrefix:
						configureXLabelPrefix(packet->id, packet->data.str);
						break;
					case kPacketConfigureYLabelPrefix:
						configureYLabelPrefix(packet->id, packet->data.str);
						break;
					case kPacketConfigureXLabelUnits:
						configureXLabelUnits(packet->id, packet->data.str);
						break;
					case kPacketConfigureYLabelUnits:
						configureYLabelUnits(packet->id, packet->data.str);
						break;
					case kPacketConfigureXLabelPrecision:
						configureXLabelPrecision(packet->id, packet->data.ival);
						break;
					case kPacketConfigureYLabelPrecision:
						configureYLabelPrecision(packet->id, packet->data.ival);
						break;
					case kPacketUpdateXLabel:
						updateXLabelValue(packet->id, packet->data.dval);
						break;
					case kPacketUpdateYLabel:
						updateYLabelValue(packet->id, packet->data.dval);
						break;
					case kPacketQuit:
						_runThread = false;
						break;
					default:
						reportError("listenerLoop: Invalid packet type\n", false);
						_runThread = false;
						break;
				}
			}
		} while (result > 0);
		usleep(kSleepMsec * 1000L);
	}
	delete [] packet;

	QuitApplicationEventLoop();

	return NULL;
}

int createListenerThread()
{
	_runThread = true;
	int retcode = pthread_create(&_listenerThread, NULL, listenerLoop, NULL);
	if (retcode != 0)
		return reportError("createListenerThread", true);
	return 0;
}

// Open new socket and, as server, block while listening for connection
// to RTcmix.  Returns 0 if connection accepted, -1 if any other error.
int openSocket()
{
	_servdesc = socket(AF_INET, SOCK_STREAM, 0);
	if (_servdesc < 0)
		return reportError("openSocket (socket)", true);

	int val = sizeof(MouseSockPacket);
	int optlen = sizeof(int);
	if (setsockopt(_servdesc, SOL_SOCKET, SO_RCVBUF, &val, optlen) < 0)
		return reportError("openSocket (setsockopt)", true);

	// This allows us to bind to the same address as last time, even if
	// the kernel is still keeping the old binding active.  In practical
	// terms, this means we can quit the MouseWindow program while RTcmix
	// is running, and then the next launch of MouseWindow will not fail
	// with "Address already in use."  It takes a minute or so for the
	// kernel to relinquish the old binding, so that can be annoying.
	val = 1;
	optlen = sizeof(int);
	if (setsockopt(_servdesc, SOL_SOCKET, SO_REUSEADDR, &val, optlen) < 0)
		return reportError("openSocket (setsockopt)", true);

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(_sockport);

	if (bind(_servdesc, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		return reportError("openSocket (bind)", true);

	if (listen(_servdesc, 1) < 0)
		return reportError("openSocket (listen)", true);

#ifdef JAGUAR
	int len = sizeof(servaddr);
#else
	socklen_t len = sizeof(servaddr);
#endif
	_newdesc = accept(_servdesc, (struct sockaddr *) &servaddr, &len);
	if (_newdesc < 0)
		return reportError("openSocket (accept)", true);

	return 0;
}

void sendQuit()
{
	MouseSockPacket packet;
	packet.type = kPacketQuit;
	writePacket(&packet);
}

int closeSocket()
{
	sendQuit();

	if (_servdesc > -1) {
		if (close(_servdesc) == -1)
			return reportError("closeSocket", true);
		_servdesc = -1;
	}
	if (_newdesc > -1) {
		if (close(_newdesc) == -1)
			return reportError("closeSocket", true);
		_newdesc = -1;
	}

	return 0;
}

// XXX have to clear memory and reinit if we want to accept a connection from
// another RTcmix run.  But for now, we quit MouseWindow each time.
void initdata(bool reinit);
void initdata(bool reinit)
{
	if (reinit) {
		for (int i = 0; i < kNumLabels; i++) {
			delete [] _xprefix[i];
			delete [] _yprefix[i];
			delete [] _xunits[i];
			delete [] _yunits[i];
			delete [] _xlabel[i];
			delete [] _ylabel[i];
		}
	}

	_xlabelCount = 0;
	_ylabelCount = 0;
	for (int i = 0; i < kNumLabels; i++) {
		_xprefix[i] = NULL;
		_yprefix[i] = NULL;
		_xunits[i] = NULL;
		_yunits[i] = NULL;
		_xlabel[i] = NULL;
		_ylabel[i] = NULL;
	}

	_servdesc = -1;
	_newdesc = -1;
}

int initialize()
{
	initdata(false);

	if (openSocket() != 0)		// Make server listen asap
		return -1;
	if (createApp() != 0)
		return -1;
#ifdef NOTYET
	if (createMenus() != 0)
		return -1;
#endif
	if (createWindow() != 0)
		return -1;
	if (createListenerThread() != 0)
		return -1;

	// Do only now, in case we got additional setup info over socket.
	ShowWindow(_window);

	return 0;
}

int finalize()
{
	_runThread = false;
	pthread_join(_listenerThread, NULL);
	return closeSocket();
}

int main()
{
	if (initialize() != 0)
		return -1;
	RunApplicationEventLoop();
	return finalize();
}

