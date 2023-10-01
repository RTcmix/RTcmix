#include <Instrument.h>      // the base class for this instrument

class FEEDBACK : public Instrument {
public:
	FEEDBACK();
	virtual ~FEEDBACK() {}
protected:
	int _bufIndex, _branch;
	float _amp;
};

class FBRECEIVE : public FEEDBACK {
public:
	FBRECEIVE();
	virtual ~FBRECEIVE();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();

	float *_feedBuffer;
};

class FBSEND : public FEEDBACK {
public:
	FBSEND();
	virtual ~FBSEND();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();

private:
	void doupdate();
	
	float *_in, *_feedBuffer;
};
