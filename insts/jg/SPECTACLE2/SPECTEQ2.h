// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "SPECTACLE2_BASE.h"

class SPECTEQ2 : public SPECTACLE2_BASE {

public:
	SPECTEQ2();
	virtual ~SPECTEQ2();

protected:
	virtual int getargs(double p[], int n_args);
	virtual int subinit(double p[], int n_args);
	virtual int subconfigure();
	virtual void subupdate();
	virtual void modify_analysis(bool reading_input);
	virtual const char *instname() { return "SPECTEQ2"; }

private:
	int usage();
	void dump_anal_bins();

	float _eqconst;
	double *_eqtable;
};

