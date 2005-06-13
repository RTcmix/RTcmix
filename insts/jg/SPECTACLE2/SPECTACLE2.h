// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

#include "SPECTACLE2_BASE.h"

class SPECTACLE2 : public SPECTACLE2_BASE {
public:
	SPECTACLE2();
	virtual ~SPECTACLE2();

protected:
	virtual int getargs(double p[], int n_args);
	virtual int subinit(double p[], int n_args);
	virtual int subconfigure();
	virtual void subupdate();
	virtual void modify_analysis(bool reading_input);
	virtual const char *instname() { return "SPECTACLE2"; }

private:
	int usage();
	void dump_anal_bins();
	bool set_eq_freqrange(float min, float max);

	long _maxdelsamps;
	float _eqconst, _deltimeconst, _feedbackconst, _eq_minfreq, _eq_rawmaxfreq;
	double *_eqtable, *_deltimetable, *_feedbacktable;
	int *_eq_bin_groups, _eqtablen;
	Odelay *_phase_delay[(kMaxFFTLen / 2) + 1];
	Odelay *_mag_delay[(kMaxFFTLen / 2) + 1];
};

