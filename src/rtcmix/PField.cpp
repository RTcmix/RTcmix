/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
 
#include "PField.h"

// PField

PField::~PField() {}

// ConstPField

ConstPField::ConstPField(double value) : _value(value) {}

ConstPField::~ConstPField() {}

double ConstPField::doubleValue(double) const
{
	return _value;
}

// TablePField

TablePField::TablePField(double *tableArray, int length)
	: _table(tableArray), _len(length)
{
}

TablePField::~TablePField()
{
	delete [] _table;
}

inline int min(int x, int y) { return (x < y) ? x : y; }

double TablePField::doubleValue(double percent) const
{
	if (percent > 1.0)
		percent = 1.0;
	double didx = (_len - 1) * percent;
	int idx = int(didx);
	int idx2 = min(idx + 1, _len - 1);
	double frac = didx - idx;
	return _table[idx] + frac * (_table[idx] - _table[idx2]);
}


