/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
 
#ifndef _PFIELD_H_
#define _PFIELD_H_

#include <RefCounted.h>

// Base class for all PFields.  Value can be retrieved at any time in any
// of the 4 supported formats.

class PField : public RefCounted {
public:
	virtual double 	doubleValue(double dindex) const = 0;
	int 			intValue(double dindex) const { return (int) doubleValue(dindex); }
	const char * 	stringValue(double dindex) const { return (const char *) intValue(dindex); }
protected:
	virtual 		~PField();
};

// Constant-value PField used for all non-varying parameters.

class ConstPField : public PField {
public:
	ConstPField(double value);
	virtual double	doubleValue(double) const;
protected:
	virtual 		~ConstPField();
private:
	double	_value;
};

// Base class for all Real-Time-varying parameters.
class RTPField : public PField {
public:
protected:
	virtual ~RTPField() {}
};

class TablePField : public RTPField {
public:
	TablePField(double *tableArray, int length);
	virtual double	doubleValue(double) const;
protected:
	virtual ~TablePField();
private:
	double *_table;
	int 	_len;
};

#endif	// _PFIELD_H_

