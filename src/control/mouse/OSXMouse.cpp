/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <OSXMouse.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

//#define DEBUG

#define LABEL_FROM_LEFT	10		// pixels from left border
#define LABEL_FROM_TOP	10
#define LABEL_FONT_NAME	"fixed"


OSXMouse::OSXMouse() : RTcmixMouse()
{
	_window = NULL;
	_xraw = -INT_MAX;		// forces getpos* to return negative
	_yraw = INT_MAX;
	_labelXpos = LABEL_FROM_LEFT;
	_labelYpos = LABEL_FROM_TOP;
	_maxLabelChars = LABEL_LENGTH;	// defined in base class
	_lineHeight = 0;
	_charWidth = 0;
	_fontName = LABEL_FONT_NAME;
	_windowname = "RTcmix Mouse Input";
}

OSXMouse::~OSXMouse()
{
	DisposeWindow(_window);
}

int OSXMouse::show()
{
	InitCursor();
	_cursor = GetCursor(crossCursor);
	if (_cursor == NULL)
		return -1;

	const int xpos = 100;
	const int ypos = 100;
	const int width = 200;
	const int height = 200;
	_window = createWindow(xpos, ypos, width, height);
	if (_window != NULL) {
		setFactors();           // must do after creating window
		return 0;
	}
	return -1;
}

WindowRef OSXMouse::createWindow(
		const int xpos,
		const int ypos,
		const int width,
		const int height)
{
	const unsigned int borderwidth = 0;

	Rect rect;
	SetRect(&rect, ypos, xpos, ypos + height, xpos + width);

	WindowRef wref;
	OSStatus result = CreateNewWindow(kDocumentWindowClass,
								kWindowStandardDocumentAttributes,
								&rect, &wref); 
	if (result != noErr) {
		fprintf(stderr, "Error creating mouse window.\n");
		return NULL;
	}

#ifdef NOMORE
	XSetWindowAttributes attr;
	attr.backing_store = WhenMapped;
	attr.event_mask = ExposureMask | PointerMotionMask | ButtonMotionMask
				| ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;
	attr.background_pixel = XWhitePixel(_display, _screen);
	const unsigned long valuemask = CWBackingStore | CWEventMask | CWBackPixel
							| CWCursor;


	XFontStruct *font = XLoadQueryFont(_display, _fontName);
	if (font == NULL) {
		fprintf(stderr, "Mouse window font not found.\n");
		return None;
	}
	XSetFont(_display, _gc, font->fid);
	_charWidth = XTextWidth(font, "X", 1);
   _lineHeight = font->ascent + font->descent;
   _fontAscent = font->ascent;

	XStoreName(_display, window, _windowname);
	ShowWindow(_display, window);
#endif

	return wref;
}

void OSXMouse::setFactors()
{
	Rect rect;
	GetWindowBounds(_window, kWindowContentRgn, &rect);
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	_xfactor = 1.0 / (double) (width - 1);
	_yfactor = 1.0 / (double) (height - 1);
}

void OSXMouse::drawXLabels()
{
	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(_window); // FIXME: how do I get GrafPtr from WindowRef?

	if (_xlabelCount > 0) {
		// Clear rect enclosing all X labels.
		int height = _xlabelCount * _lineHeight;
		int width = _maxLabelChars * _charWidth;
		int ypos = _labelYpos;
		Rect rect;
		SetRect(&rect, ypos, _labelXpos, ypos + height, _labelXpos + width);
		EraseRect(&rect);
#ifdef DEBUG
		FrameRect(&rect);
		printf("drawXLabel: xpos=%d, ypos=%d, width=%d, height=%d\n",
					_labelXpos, ypos, width, height);
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
	}
	SetPort(oldPort);
}

void OSXMouse::drawYLabels()
{
	if (_ylabelCount > 0) {
		// Clear rect enclosing all Y labels.
		int height = _ylabelCount * _lineHeight;
		int width = _maxLabelChars * _charWidth;
		int ypos = _labelYpos + (_xlabelCount * _lineHeight);
		XClearArea(_display, _window, _labelXpos, ypos, width, height, False);
#ifdef DEBUG
		XDrawRectangle(_display, _window, _gc, _labelXpos, ypos, width, height);
		printf("drawYLabel: xpos=%d, ypos=%d, width=%d, height=%d\n",
					_labelXpos, ypos, width, height);
#endif

		// Draw all Y labels.
		ypos += _fontAscent;
		int line = 0;
		for (int i = 0; i < _ylabelCount; i++) {
			XDrawString(_display, _window, _gc,
							_labelXpos, ypos + (line * _lineHeight),
							_ylabel[i], strlen(_ylabel[i]));
			line++;
		}
		XFlush(_display);
	}
}

void OSXMouse::drawWindowContent()
{
	drawXLabels();
	drawYLabels();
}

bool OSXMouse::handleEvents()
{
	XEvent event;
	const unsigned long evtmask =
		  ExposureMask
		| PointerMotionMask
		| StructureNotifyMask;

	bool keepgoing = true;
	while (XCheckWindowEvent(_display, _window, evtmask, &event)) {
		switch (event.type) {
			case MotionNotify:
				_xraw = event.xmotion.x;
				_yraw = event.xmotion.y;
				break;
			case Expose:
				drawWindowContent();
				break;
			case ConfigureNotify:
				setFactors();
				break;
			default:
				break;
		}
	}
	return keepgoing;
}

