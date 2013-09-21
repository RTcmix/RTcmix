// RVB.h--	Post-processing reverb.  Takes dry signal from normal input and
// 			summed first and second generation reflections from a global array

#ifndef _RTCMIX_RVB_H_
#define _RTCMIX_RVB_H_

#include <Instrument.h>

#define NCOEFFS   512
#define NPRIMES   5000
#define MAX_INPUTS  4

class RVB : public Instrument {
public:
	RVB();
	virtual ~RVB();
	virtual int init(double *, int);
	virtual int configure();
	virtual int run();
protected:
	void set_random(void);
	void get_lengths(long);
	void set_gains(float);
	void alloc_delays(void);
	void set_allpass(void);
	void wire_matrix(double [12][12]);
	void rvb_reset();
	void doRun(double *, double *, long);	// was BASE::RVB()
	void matrix_mix();
private:
	struct ReverbPatch {
		int incount;
		double *inptrs[MAX_INPUTS];
		double gains[MAX_INPUTS];
		double *outptr;
	} ReverbPatches[12];
	struct ReverbData {
		double delin;
		double Rand_info[6];
		double *Rvb_del;
		int deltap;
		double Rvb_air[3];
		double delout;
	} m_rvbData[2][6];
	double		Allpass_del[2][502];
	int			allpassTap[2];
	double		m_rvbPast[2];	// For hi-pass filter
	double		AIRCOEFFS[NCOEFFS];
	double		Dimensions[5];
	double		Nsdelay[2][6];
	float		m_dur, m_amp;
	float		*in;
	int			insamps, rvbdelsize;
	int			_branch;
	// static methods
	static void get_primes(int x, int p[]);
	// static data
	static int	primes[NPRIMES + 2];
	static AtomicInt primes_gotten;
};

#endif	// _RTCMIX_RVB_H_

