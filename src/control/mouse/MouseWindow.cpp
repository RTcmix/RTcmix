/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// A simple Carbon application that receives setup information from RTcmix
// over a socket and sends mouse coordinates back.  It's supposed to support
// the same functionality as the version for X.
//
// -John Gibson, 12/07/04

// XXX move to Makefile: g++ -g -o MouseWindow MouseWindow.cpp -framework Carbon

#include <Carbon/Carbon.h>
#include <iostream.h>

#define DEBUG

// FIXME: move these into commond hdr
#define LABEL_FROM_LEFT	10		// pixels from left border
#define LABEL_FROM_TOP	10
// XXX: must redefine from hdr
#define LABEL_FONT_NAME	"Monoco"
#define LABEL_FONT_SIZE	10
#define LABEL_FONT_FACE	0		// i.e., plain
#define LABEL_LENGTH		32
#define NLABELS			4		// Number of labels per axis

const int _titleBarHeight = 22;	// FIXME: should get this from system
const int _labelXpos = LABEL_FROM_LEFT;
const int _labelYpos = LABEL_FROM_TOP;
const int _maxLabelChars = LABEL_LENGTH;

char *_xlabel[NLABELS];
char *_ylabel[NLABELS];
int _xlabelCount = 1;
int _ylabelCount;

int _lineHeight = 0;
int _charWidth = 0;
int _fontAscent = 0;
double _xfactor;
double _yfactor;
WindowRef _window;

// Default window position and size
enum {
	kWindowXpos = 100,
	kWindowYpos = 100,
	kWindowWidth = 300,
	kWindowHeight = 300
};


// =============================================================================
// Utilities

int reportConsoleError(const char *err)
{
	cout << err << endl;
	return -1;
}

int reportError(const char *err)
{
//FIXME: pop alert instead of console print.
	reportConsoleError(err);
	return -1;
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
// FIXME: probably must subtract window title bar height?
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
cout << "kEventWindowDrawContent" << endl;
			drawWindowContent();
			status = noErr;
			break;
		case kEventWindowBoundsChanged:
cout << "kEventWindowBoundsChanged" << endl;
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

void sendCoordinates(int x, int y)
{
}

pascal OSStatus doWindowMouseMoved(EventHandlerCallRef nextHandler,
	EventRef theEvent, void *userData)
{
	Point mouseLoc;
	GetEventParameter(theEvent, kEventParamWindowMouseLocation, typeQDPoint,
		NULL, sizeof(Point), NULL, &mouseLoc);

	int x = mouseLoc.h;
	int y = mouseLoc.v - _titleBarHeight;
 
//	cout << "x: " << x << " y: " << y << endl;

	if (y >= 0) {

// print cooked mouse coords
// send them back to RTcmix first
		sendCoordinates(x, y);
		drawXLabels();
		drawYLabels();

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
		return reportError("Can't install app mouse event handler.");

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
		return reportConsoleError("Can't create file menu.");
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
		return reportError("Can't create window.");

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
		return reportError("Can't install window close event handler.");

	numTypes = 1;
	eventTypes[0].eventClass = kEventClassMouse;
	eventTypes[0].eventKind = kEventMouseMoved;
	status = InstallWindowEventHandler(_window,
					NewEventHandlerUPP(doWindowMouseMoved),
					numTypes, eventTypes, NULL, NULL);
	if (status != noErr)
		return reportError("Can't install window mouse event handler.");

	// Get font info.
	SetPort(GetWindowPort(_window));

	// NB: This is the deprecated way, but the new way seems too complicated.
	// We'll figure it out when it's really necessary.
	Str255 str;
	CopyCStringToPascal(LABEL_FONT_NAME, str);
	SInt16 fontID;
	GetFNum(str, &fontID);
	if (fontID == 0)
		fontID = applFont;
	TextFont(fontID);
	TextSize(LABEL_FONT_SIZE);
	TextFace(LABEL_FONT_FACE);
	FontInfo finfo;
	GetFontInfo(&finfo);
	_charWidth = finfo.widMax;
	_lineHeight = finfo.ascent + finfo.descent;
	_fontAscent = finfo.ascent;

_xlabel[0] = "banana";

	return 0;
}

int openSocket()
{
	return 0;
}

int closeSocket()
{
	return 0;
}

int initialize()
{
	if (createApp() != 0)
		return -1;
#ifdef NOTYET
	if (createMenus() != 0)
		return -1;
#endif
	if (createWindow() != 0)
		return -1;
	if (openSocket() != 0)
		return -1;

	// Do only now, in case we got additional setup info over socket.
	ShowWindow(_window);

	return 0;
}

int finalize()
{
	return closeSocket();
}

int main()
{
	if (initialize() != 0)
		return -1;
	RunApplicationEventLoop();
	return finalize();
}

