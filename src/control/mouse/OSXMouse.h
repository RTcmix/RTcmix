/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _OSXMOUSE_H_
#define _OSXMOUSE_H_
#include <RTcmixMouse.h>
#include <Carbon/Carbon.h>

class OSXMouse : public RTcmixMouse {
public:
	OSXMouse();
	virtual ~OSXMouse();

	virtual int show();

protected:
	// RTcmixMouse reimplementations

	virtual inline double getPositionX() const
	{
		return _xraw * _xfactor;
	}

	virtual inline double getPositionY() const
	{
		return 1.0 - (_yraw * _yfactor);
	}

	virtual bool handleEvents();
	virtual void drawXLabels();
	virtual void drawYLabels();

private:
	WindowRef createWindow(const int xpos, const int ypos,
					const int width, const int height);
	void setFactors();
	void drawWindowContent();

	WindowRef _window;
	char *_windowname;
	CursHandle _cursor;

	int _xraw;
	int _yraw;
	double _xfactor;
	double _yfactor;

	char *_fontName;
	int _labelXpos;
	int _labelYpos;
	int _fontAscent;
	int _lineHeight;
	int _maxLabelChars;
	int _charWidth;
};

#endif // _OSXMOUSE_H_
