/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
 
#include <PField.h>
#include <string.h>
#include <stdio.h>   // for snprintf
#include <math.h>

inline int max(int x, int y) { return (x >= y) ? x : y; }
inline int min(int x, int y) { return (x < y) ? x : y; }

// PField

PField::~PField() {}

// Return a string version of the pfield value if the pointer address seems
// to be valid, else NULL (which means the PField did not contain a string).

const char * 	
PField::stringValue(double dindex) const
{
	const int ivalue = intValue(dindex);
	return (const char *) ((ivalue < 0x100000) ? 0 : ivalue);
}

int PField::print(FILE *file) const
{
	int chars = 0;
	for (int n = 0; n < values(); ++n)
		chars += fprintf(file, "%.6f\n", doubleValue(n));
	return chars;
}

int PField::copyValues(double *array) const
{
	const int len = values();
	for (int n = 0; n < len; ++n)
		array[n] = doubleValue(n);
	return len;
}

// SingleValuePField

double SingleValuePField::doubleValue(double) const
{
	return _value;
}

// ConstPField

ConstPField::ConstPField(double value) : SingleValuePField(value) {}

ConstPField::~ConstPField() {}


// StringPField

StringPField::StringPField(const char *value)
	: _string(new char [strlen(value) + 1])
{
	strcpy(_string, value);
}

StringPField::~StringPField() { delete [] _string; }

// Note:  We use the same old trick here to pass the string as a double,
// but the conversion is entirely contained within the PField class system.
// PField::stringValue() casts it back to a const char *.

double StringPField::doubleValue(double) const
{
	return (double) (int) _string;
}

int StringPField::print(FILE *file) const
{
	return fprintf(file, "\"%s\"\n", _string);
}

// RTNumberPField

RTNumberPField::RTNumberPField(double value) : SingleValuePField(value) {}

double RTNumberPField::set(double value) { return setValue(value); }

// PFieldBinaryOperator

PFieldBinaryOperator::PFieldBinaryOperator(PField *pf1, PField *pf2,
										   PFieldBinaryOperator::Operator op)
	: _pfield1(pf1), _pfield2(pf2), _operator(op)
{
	_pfield1->ref();
	_pfield2->ref();
}

PFieldBinaryOperator::~PFieldBinaryOperator()
{
	_pfield2->unref();
	_pfield1->unref();
}

double PFieldBinaryOperator::doubleValue(int indx) const
{
	const int rindx = min(indx, values() - 1);
	return (*_operator)(_pfield1->doubleValue(rindx), _pfield2->doubleValue(rindx));
}

double PFieldBinaryOperator::doubleValue(double frac) const
{
	return (*_operator)(_pfield1->doubleValue(frac), _pfield2->doubleValue(frac));
}

int PFieldBinaryOperator::values() const
{
	const int len1 = _pfield1->values();
	const int len2 = _pfield2->values();
	return max(len1, len2);
}

int PFieldBinaryOperator::print(FILE *file) const
{
	const int len1 = _pfield1->values();
	const int len2 = _pfield2->values();
	int maxlen;
	PField *mintable, *maxtable;
	if (len1 >= len2) {
		maxlen = len1;
		maxtable = _pfield1;
		mintable = _pfield2;
	}
	else {
		maxlen = len2;
		maxtable = _pfield2;
		mintable = _pfield1;
	}
	int chars = 0;
	for (int n = 0; n < maxlen; ++n) {
		double frac = (double) n/(maxlen-1);
		double value = (*_operator)(maxtable->doubleValue(n), mintable->doubleValue(frac));
		chars += fprintf(file, "%.6f\n", value);
	}
	return chars;
}

int PFieldBinaryOperator::copyValues(double *array) const
{
	const int len1 = _pfield1->values();
	const int len2 = _pfield2->values();
	int maxlen;
	PField *mintable, *maxtable;
	if (len1 >= len2) {
		maxlen = len1;
		maxtable = _pfield1;
		mintable = _pfield2;
	}
	else {
		maxlen = len2;
		maxtable = _pfield2;
		mintable = _pfield1;
	}
	int chars = 0;
	for (int n = 0; n < maxlen; ++n) {
		double frac = (double) n/(maxlen-1);
		double value = (*_operator)(maxtable->doubleValue(n), mintable->doubleValue(frac));
		array[n] = value;
	}
	return maxlen;
}

// LFOPField

LFOPField::LFOPField(double krate, double *tableArray, int length,
		PField *freq, PField *amp, LFOPField::InterpFunction ifun)
	: SingleValuePField(0.0), _freqPF(freq), _ampPF(amp), _interpolator(ifun)
{
	_oscil = new Ooscili(krate, _freqPF->doubleValue(0), tableArray, length);
}

LFOPField::~LFOPField()
{
	delete _oscil;
}

double LFOPField::Truncate(Ooscili *oscil)
{
	return oscil->nextn();
}

double LFOPField::Interpolate1stOrder(Ooscili *oscil)
{
	return oscil->next();
}

double LFOPField::doubleValue(double percent) const
{
	if (percent > 1.0)
		percent = 1.0;
	_oscil->setfreq(_freqPF->doubleValue(percent));
	return (*_interpolator)(_oscil) * _ampPF->doubleValue(percent);
}

// TablePField

TablePField::TablePField(double *tableArray,
						 int length,
						 TablePField::InterpFunction ifun)
	: _table(tableArray), _len(length), _interpolator(ifun)
{
}

TablePField::~TablePField()
{
	delete [] _table;
}

double TablePField::Truncate(double *tab, int len, double didx)
{
	const int idx = int(didx);
	return tab[idx];
}

double TablePField::Interpolate1stOrder(double *tab, int len, double didx)
{
	const int idx = int(didx);
	const int idx2 = min(idx + 1, len - 1);
	double frac = didx - idx;
	return tab[idx] + frac * (tab[idx2] - tab[idx]);
}

double TablePField::Interpolate2ndOrder(double *tab, int len, double didx)
{
	//FIXME: this is bound to be all wrong  -JGG
	const int idx = int(didx);
	const int idx2 = min(idx + 1, len - 1);
	const int idx3 = min(idx + 2, len - 1);
	double frac = didx - idx;
	double a = tab[idx];
	double hy0 = a / 2.0;
	double hy2 = tab[idx3] / 2.0;
	double b = (-3.0 * hy0) + (2.0 * tab[idx2]) - hy2;
	double c = hy0 - tab[idx2] + hy2;
	return a + (b * frac) + (c * frac * frac);
}

double TablePField::doubleValue(int indx) const
{
	return _table[min(indx, values() - 1)];
}

double TablePField::doubleValue(double percent) const
{
	if (percent > 1.0)
		percent = 1.0;
	const int len = values();
	double didx = (len - 1) * percent;
	return (*_interpolator)(_table, len, didx);
}

int TablePField::print(FILE *file) const
{
	int chars = 0;
	for (int i = 0; i < values(); i++) {
		chars += fprintf(file, "%d %.6f\n", i, _table[i]);
	}
	return chars;
}

// Optimized version for table

int TablePField::copyValues(double *array) const
{
	const int len = values();
	for (int n = 0; n < len; ++n)
		array[n] = _table[n];
	return len;
}

// PFieldWrapper

PFieldWrapper::PFieldWrapper(PField *innerPField) : _pField(innerPField)
{
	_pField->ref();
}

PFieldWrapper::~PFieldWrapper() { _pField->unref(); }

// LoopedPField

LoopedPField::LoopedPField(PField *innerPField, double loopFactor)
	: PFieldWrapper(innerPField), _factor(loopFactor), _len(innerPField->values())
{
}

double LoopedPField::doubleValue(double didx) const
{
	double dfrac = didx * _factor;
	while (dfrac > 1.0)
		dfrac -= 1.0;
	return field()->doubleValue(dfrac);
}  

double LoopedPField::doubleValue(int idx) const
{
	int nidx = int(idx * _factor);
	while (nidx >= _len)
		nidx -= _len;
	return field()->doubleValue(nidx);
}

// ReversePField

ReversePField::ReversePField(PField *innerPField)
	: PFieldWrapper(innerPField), _len(innerPField->values())
{
}

double ReversePField::doubleValue(double didx) const
{
	return field()->doubleValue(1.0 - didx);
}  

double ReversePField::doubleValue(int idx) const
{
	return field()->doubleValue((_len - 1) - idx);
}

// ConverterPField

ConverterPField::ConverterPField(PField *innerPField,
					 ConverterPField::ConverterFunction cfun)
	: PFieldWrapper(innerPField), _converter(cfun)
{
}

double ConverterPField::doubleValue(double percent) const
{
	double val = field()->doubleValue(percent);
	return (*_converter)(val);
}  

double ConverterPField::doubleValue(int idx) const
{
	double val = field()->doubleValue(idx);
	return (*_converter)(val);
}  

#include <ugens.h>

double ConverterPField::ampdb(const double db)
{
	return ::ampdb(db);
}

double ConverterPField::cpsoct(const double oct)
{
	return ::cpsoct(oct);
}

double ConverterPField::octpch(const double pch)
{
	return ::octpch(pch);
}

double ConverterPField::cpspch(const double pch)
{
	return ::cpspch(pch);
}

double ConverterPField::pchoct(const double oct)
{
	return ::pchoct(oct);
}

double ConverterPField::pchmidi(const double midi)
{
	return ::pchmidi((unsigned char) midi);
}

// NB: returns an amp multiplier; feed this to an amp pfield, not a pan pfield
double ConverterPField::boost(const double pan)
{
	return ::boost(pan);
}

