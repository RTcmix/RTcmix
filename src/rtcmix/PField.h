/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
 
#ifndef _PFIELD_H_
#define _PFIELD_H_

#include <RefCounted.h>
#include <stdio.h>

// Base class for all PFields.  Value can be retrieved at any time in any
// of the 4 supported formats.

class PField : public RefCounted {
public:
	virtual double 	doubleValue(double dindex=0.0) const = 0;
	int 			intValue(double dindex) const { return (int) doubleValue(dindex); }
	const char * 	stringValue(double dindex) const { return (const char *) intValue(dindex); }
	virtual int		print(FILE *) const;
protected:
	virtual 		~PField();
};

// Base class for operator PFields.  These may be instantianted with an
// external Operator function.

class PFieldBinaryOperator : public PField {
public:
	virtual double	doubleValue(double) const;
	typedef double (*Operator)(double, double);
	PFieldBinaryOperator(PField *pf1, PField *pf2, Operator);
protected:
	virtual 		~PFieldBinaryOperator();
private:
	PField	*_pfield1, *_pfield2;
	Operator _operator;
};

// PField which adds two other PFields

class AddPField : public PFieldBinaryOperator {
protected:
	static double Add(double x, double y) { return x + y; }
public:
	AddPField(PField *pf1, PField *pf2) : PFieldBinaryOperator(pf1, pf2, Add) {}
protected:
	virtual 		~AddPField() {}
};

// PField which multiplies two other PFields

class MultPField : public PFieldBinaryOperator {
protected:
	static double Mult(double x, double y) { return x * y; }
public:
	MultPField(PField *pf1, PField *pf2) : PFieldBinaryOperator(pf1, pf2, Mult) {}
protected:
	virtual 		~MultPField() {}
};

// Constant-value PField used for all non-varying numeric parameters.

class ConstPField : public PField {
public:
	ConstPField(double value);
	virtual double	doubleValue(double) const;
protected:
	virtual 		~ConstPField();
private:
	double	_value;
};

// Constant-value PField used for all strings.

class StringPField : public PField {
public:
	StringPField(const char  *value);
	virtual double	doubleValue(double) const;
protected:
	virtual 		~StringPField();
private:
	char	*_string;
};

// Base class for all Real-Time-varying parameters.

class RTPField : public PField {
public:
protected:
	virtual ~RTPField() {}
};

// Class for interpolated reading of table.

class TablePField : public RTPField {
public:
	TablePField(double *tableArray, int length);
	virtual double	doubleValue(double) const;
	virtual int		print(FILE *) const;	// redefined
protected:
	virtual ~TablePField();
	int		len() const { return _len; }
private:
	double	*_table;
	int 	_len;
};

#endif	// _PFIELD_H_

