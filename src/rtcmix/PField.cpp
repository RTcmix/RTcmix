/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
 
#include <PField.h>
#include <string.h>
#include <stdio.h>   // for snprintf
#include <math.h>
#include <float.h>
#include <Ougens.h>
#include <Functor.h>

inline int max(int x, int y) { return (x >= y) ? x : y; }
inline int min(int x, int y) { return (x < y) ? x : y; }

// PField

#ifdef DEBUG
PField::PField()
{
	this;
}
#endif

PField::~PField()
{
#ifdef DEBUG
	printf("PField::~PField (this = 0x%x)\n", this);
#endif
}

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
	int chars = 0;
	if (len1 >= len2) {
		for (int n = 0; n < len1; ++n) {
			double frac = (double) n / (len1 - 1);
			double value = (*_operator)(_pfield1->doubleValue(n), _pfield2->doubleValue(frac));
			chars += fprintf(file, "%.6f\n", value);
		}
	}
	else {
		for (int n = 0; n < len2; ++n) {
			double frac = (double) n / (len2 - 1);
			double value = (*_operator)(_pfield1->doubleValue(frac), _pfield2->doubleValue(n));
			chars += fprintf(file, "%.6f\n", value);
		}
	}
	return chars;
}

int PFieldBinaryOperator::copyValues(double *array) const
{
	const int len1 = _pfield1->values();
	const int len2 = _pfield2->values();
	if (len1 >= len2) {
		for (int n = 0; n < len1; ++n) {
			double frac = (double) n / (len1 - 1);
			double value = (*_operator)(_pfield1->doubleValue(n), _pfield2->doubleValue(frac));
			array[n] = value;
		}
		return len1;
	}
	else {
		for (int n = 0; n < len2; ++n) {
			double frac = (double) n / (len2 - 1);
			double value = (*_operator)(_pfield1->doubleValue(frac), _pfield2->doubleValue(n));
			array[n] = value;
		}
		return len2;
	}
}

// LFOPField

LFOPField::LFOPField(double krate, TablePField *tablePField,
		PField *freq, LFOPField::InterpFunction ifun)
	: SingleValuePField(0.0), _tablePF(tablePField), _freqPF(freq), _interpolator(ifun)
{
	_tablePF->ref();
	_freqPF->ref();
	double *table = (double *) *_tablePF;
	const int length = _tablePF->values();
	_oscil = new Ooscil(krate, _freqPF->doubleValue(0), table, length);
}

LFOPField::~LFOPField()
{
	delete _oscil;
	_freqPF->unref();
	_tablePF->unref();
}

double LFOPField::Truncate(Ooscil *oscil)
{
	return oscil->next();
}

double LFOPField::Interpolate1stOrder(Ooscil *oscil)
{
	return oscil->nexti();
}

double LFOPField::doubleValue(double percent) const
{
	if (percent > 1.0)
		percent = 1.0;
	_oscil->setfreq(_freqPF->doubleValue(percent));
	return (*_interpolator)(_oscil);
}

// RandomPField

#include <Random.h>

RandomPField::RandomPField(double krate, Random *generator, PField *freq,
		PField *min, PField *max, PField *mid, PField *tight)
	: SingleValuePField(0.0), _freqPF(freq), _minPF(min), _maxPF(max), _midPF(mid), _tightPF(tight)
{
	_freqPF->ref();
	_minPF->ref();
	_maxPF->ref();
	RefCounted::ref(_tightPF);
	RefCounted::ref(_midPF);
	_randOscil = new RandomOscil(generator, krate, _freqPF->doubleValue());
}

RandomPField::~RandomPField()
{
	delete _randOscil;
	RefCounted::unref(_tightPF);
	RefCounted::unref(_midPF);
	_maxPF->unref();
	_minPF->unref();
	_freqPF->unref();
}

double RandomPField::doubleValue(double percent) const
{
	if (percent > 1.0)
		percent = 1.0;
	_randOscil->setmin(_minPF->doubleValue(percent));
	_randOscil->setmax(_maxPF->doubleValue(percent));
	if (_midPF)
		_randOscil->setmid(_midPF->doubleValue(percent));
	if (_tightPF)
		_randOscil->settight(_tightPF->doubleValue(percent));
	_randOscil->setfreq(_freqPF->doubleValue(percent));
	return _randOscil->next();
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

PFieldWrapper::PFieldWrapper(PField *innerPField)
	: _pField(innerPField), _len(innerPField->values())
{
	_pField->ref();
}

PFieldWrapper::~PFieldWrapper() { _pField->unref(); }

// ModifiedIndexPFieldWrapper

ModifiedIndexPFieldWrapper::ModifiedIndexPFieldWrapper(PField *innerPField,
												       IIFunctor *iif, DIFunctor *dif)
	: PFieldWrapper(innerPField), _iifun(iif), _difun(dif)
{
}

ModifiedIndexPFieldWrapper::~ModifiedIndexPFieldWrapper()
{
	delete _iifun;
	delete _difun;
}

double
ModifiedIndexPFieldWrapper::doubleValue(double didx) const
{
	return field()->doubleValue((*_difun)(didx, values()));
}

double
ModifiedIndexPFieldWrapper::doubleValue(int idx) const
{
	return field()->doubleValue((*_iifun)(idx, values()));
}

// LoopedPField

#ifdef OLD	/* The code as it was as direct subclass of PFieldWrapper */

LoopedPField::LoopedPField(PField *innerPField, double loopFactor)
	: PFieldWrapper(innerPField), _factor(loopFactor)
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
	const int len = values();
	while (nidx >= len)
		nidx -= len;
	return field()->doubleValue(nidx);
}

#else

// These are the two Functors for LoopedPField - one for double indices,
// one for integer.

class LoopedPField::LoopIIFunctor : public IIFunctor {
public:
	LoopIIFunctor(double factor) : _factor(factor) {}
	virtual int operator ()(int i1, int i2);
private:
	double _factor;	
};

class LoopedPField::LoopDIFunctor : public DIFunctor {
public:
	LoopDIFunctor(double factor) : _factor(factor) {}
	virtual double operator ()(double d1, int i1);
private:
	double _factor;	
};

int LoopedPField::LoopIIFunctor::operator ()(int idx, int len)
{
	int nidx = int(idx * _factor);
	while (nidx >= len)
		nidx -= len;
	return nidx;
}

double LoopedPField::LoopDIFunctor::operator ()(double didx, int)
{
	double dfrac = didx * _factor;
	while (dfrac > 1.0)
		dfrac -= 1.0;
	return dfrac;
}  

// This is all we need to define for the actual LoopedPField class

LoopedPField::LoopedPField(PField *innerPField, double loopFactor)
	: ModifiedIndexPFieldWrapper(innerPField,
								 new LoopIIFunctor(loopFactor),
								 new LoopDIFunctor(loopFactor))
{
}

#endif	// !OLD

// ReversePField

ReversePField::ReversePField(PField *innerPField) : PFieldWrapper(innerPField)
{
}

double ReversePField::doubleValue(double didx) const
{
	return field()->doubleValue(1.0 - didx);
}

double ReversePField::doubleValue(int idx) const
{
	const int len = values();
	return field()->doubleValue((len - 1) - idx);
}

// InvertPField

InvertPField::InvertPField(PField *innerPField, PField *centerPField)
	: PFieldWrapper(innerPField), _centerPField(centerPField)
{
	_centerPField->ref();
}

InvertPField::~InvertPField()
{
	_centerPField->unref();
}

double InvertPField::doubleValue(double didx) const
{
	const double center = _centerPField->doubleValue(didx);
	const double diff = field()->doubleValue(didx) - center;
	return center - diff;
}

double InvertPField::doubleValue(int idx) const
{
	return doubleValue((double) idx);
}

// RangePField

RangePField::RangePField(PField *innerPField,
						 PField *minPField, PField *maxPField,
						 RangePField::RangeFitFunction fun)
	: PFieldWrapper(innerPField),
	  _minPField(minPField), _maxPField(maxPField), _rangefitter(fun)
{
	_minPField->ref();
	_maxPField->ref();
}

RangePField::~RangePField()
{
	_maxPField->unref();
	_minPField->unref();
}

// Assumes val is in range [0, 1]
double RangePField::UnipolarSource(const double val, const double min, const double max)
{
	return min + (val * (max - min));
}

// Assumes val is in range [-1, 1]
double RangePField::BipolarSource(const double val, const double min, const double max)
{
	return min + ((val + 1.0) * 0.5 * (max - min));
}

double RangePField::doubleValue(double didx) const
{
	const double min = _minPField->doubleValue(didx);
	const double max = _maxPField->doubleValue(didx);
	const double normval = field()->doubleValue(didx);
	return (*_rangefitter)(normval, min, max);
}

double RangePField::doubleValue(int idx) const
{
	const double min = _minPField->doubleValue(idx);
	const double max = _maxPField->doubleValue(idx);
	const double normval = field()->doubleValue(idx);
	return (*_rangefitter)(normval, min, max);
}

// SmoothPField

SmoothPField::SmoothPField(PField *innerPField, double krate, PField *lagPField)
	: PFieldWrapper(innerPField), _lagPField(lagPField)
{
	_lagPField->ref();
	_filter = new OonepoleTrack(krate);
	updateCutoffFreq();
}

SmoothPField::~SmoothPField()
{
	delete _filter;
	_lagPField->unref();
}

void SmoothPField::updateCutoffFreq(double percent) const
{
	const double lag = _lagPField->doubleValue(percent);
	_filter->setlag(lag * 0.01);
}

double SmoothPField::doubleValue(double didx) const
{
	updateCutoffFreq(didx);
	return _filter->next(field()->doubleValue(didx));
}

double SmoothPField::doubleValue(int idx) const
{
	updateCutoffFreq(idx);
	return _filter->next(field()->doubleValue(idx));
}

// QuantizePField

QuantizePField::QuantizePField(PField *innerPField, PField *quantumPField)
	: PFieldWrapper(innerPField), _quantumPField(quantumPField)
{
	_quantumPField->ref();
}

QuantizePField::~QuantizePField()
{
	_quantumPField->unref();
}

double QuantizePField::quantizeValue(const double val, const double quantum) const
{
	const double quotient = fabs(val / quantum);
	int floor = int(quotient);
	const double remainder = quotient - double(floor);
	if (remainder >= 0.5)		// round to nearest
		floor++;
	if (val < 0.0)
		return -floor * quantum;
	return floor * quantum;
}

double QuantizePField::doubleValue(double didx) const
{
	return quantizeValue(field()->doubleValue(didx), _quantumPField->doubleValue(didx));
}

double QuantizePField::doubleValue(int idx) const
{
	return quantizeValue(field()->doubleValue(idx), _quantumPField->doubleValue(idx));
}

// ClipPField

ClipPField::ClipPField(PField *innerPField, PField *minPField, PField *maxPField)
	: PFieldWrapper(innerPField), _minPField(minPField), _maxPField(maxPField)
{
	_minPField->ref();
	_maxPField->ref();
}

ClipPField::~ClipPField()
{
	_maxPField->unref();
	_minPField->unref();
}

double ClipPField::doubleValue(double didx) const
{
	double val = field()->doubleValue(didx);
	const double min = _minPField->doubleValue(didx);
	if (val < min)
		return min;
	else if (_maxPField) {
		const double max = _maxPField->doubleValue(didx);
		if (val > max)
			return max;
	}
	return val;
}

double ClipPField::doubleValue(int idx) const
{
	return doubleValue((double) idx);
}

// ConstrainPField

// helper class
Constrainer::Constrainer(const double *table, const int tableLen)
	: _table(table), _tableLen(tableLen), _lastVal(DBL_MAX), _lastTableVal(0.0)
{
}

double Constrainer::next(const double val, const double strength)
{
	if (val != _lastVal && _table) {
		double min = DBL_MAX;
		int closest = 0;
		for (int i = 0; i < _tableLen; i++) {
			const double proximity = fabs(_table[i] - val);
			if (proximity < min) {
				min = proximity;
				closest = i;
			}
		}
		_lastTableVal = _table[closest];
		_lastVal = val;
	}
	if (strength == 0.0)
		return _lastVal;
	else if (strength == 1.0)
		return _lastTableVal;
	else
		return _lastVal + ((_lastTableVal - _lastVal) * strength);
}

ConstrainPField::ConstrainPField(PField *innerPField, TablePField *tablePField,
		PField *strengthPField)
	: PFieldWrapper(innerPField),
	  _tablePField(tablePField), _strengthPField(strengthPField)
{
	_tablePField->ref();
	_strengthPField->ref();
	const double *table = (double *) *_tablePField;
	_constrainer = new Constrainer(table, _tablePField->values());
}

ConstrainPField::~ConstrainPField()
{
	delete _constrainer;
	_strengthPField->unref();
	_tablePField->unref();
}

double ConstrainPField::doubleValue(double didx) const
{
	return _constrainer->next(field()->doubleValue(didx), _strengthPField->doubleValue(didx));
}

double ConstrainPField::doubleValue(int idx) const
{
	return _constrainer->next(field()->doubleValue(idx), _strengthPField->doubleValue(idx));
}

// MapPField

// helper class
Mapper::Mapper(TablePField *tablePF, const double inputMin, const double inputMax)
	: _tablePF(tablePF), _inputMin(inputMin), _lastVal(DBL_MAX), _lastOutput(0.0)
{
	_inputDiff = inputMax - inputMin;
}

double Mapper::next(const double val)
{
	if (val != _lastVal) {
		// normalize inputVal to [0,1] and treat as table percentage
		double percent = (val - _inputMin) / _inputDiff;
		double tabval = _tablePF->doubleValue(percent);
		// map table value to input range
		_lastOutput = _inputMin + (tabval * _inputDiff);
		_lastVal = val;
	}
	return _lastOutput;
}

MapPField::MapPField(PField *innerPField, TablePField *tablePField,
		const double inputMin, const double inputMax)
	: PFieldWrapper(innerPField), _tablePField(tablePField)
{
	_tablePField->ref();
	_mapper = new Mapper(_tablePField, inputMin, inputMax);
}

MapPField::~MapPField()
{
	delete _mapper;
	_tablePField->unref();
}

double MapPField::doubleValue(double didx) const
{
	return _mapper->next(field()->doubleValue(didx));
}

double MapPField::doubleValue(int idx) const
{
	return _mapper->next(field()->doubleValue(idx));
}

// DataFileWriterPField

#include <DataFile.h>

DataFileWriterPField::DataFileWriterPField(PField *innerPField,
		const char *fileName, const bool clobber, const int controlRate,
		const int fileRate, const int format, const bool swap)
	: PFieldWrapper(innerPField)
{
	_datafile = new DataFile(fileName, controlRate);
	if (_datafile) {
		if (_datafile->openFileWrite(clobber) == 0)
			_datafile->writeHeader(fileRate, format, swap);
		else {
			delete _datafile;
			_datafile = NULL;
		}
	}
}

DataFileWriterPField::~DataFileWriterPField()
{
	delete _datafile;
}

double DataFileWriterPField::doubleValue(double didx) const
{
	const double val = field()->doubleValue(didx);
	if (_datafile) {
		if (_datafile->writeOne(val) != 0) {
			delete _datafile;
			_datafile = NULL;
		}
	}
	return val;
}

double DataFileWriterPField::doubleValue(int idx) const
{
	return doubleValue((double) idx);
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

double ConverterPField::octcps(const double cps)
{
	return ::octcps(cps);
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

double ConverterPField::pchcps(const double cps)
{
	return ::pchcps(cps);
}

double ConverterPField::pchmidi(const double midi)
{
	return ::pchmidi((unsigned char) midi);
}

double ConverterPField::octmidi(const double midi)
{
	return ::octmidi((unsigned char) midi);
}

// NB: returns an amp multiplier; feed this to an amp pfield, not a pan pfield
double ConverterPField::boost(const double pan)
{
	return ::boost(pan);
}

