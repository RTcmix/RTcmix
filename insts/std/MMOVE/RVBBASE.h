//
// Created by Douglas Scott on 4/5/25.
//

#ifndef _RVBBASE_H_
#define _RVBBASE_H_

#include <Instrument.h>
#include <rt_types.h>

#define NCOEFFS   512
#define NPRIMES   5000
#define MAX_INPUTS  4

template<int OutChannels, int DelayCount>
class RVBBASE : public Instrument {
public:
    RVBBASE();
    virtual ~RVBBASE();
    virtual int init(double *, int);
    virtual int configure();
    virtual int run();
protected:
    virtual void set_random() = 0;
    virtual void get_lengths(long) = 0;
    void set_gains(float);
    void alloc_delays();
    void set_allpass();
    void wire_matrix(double [12][12]);
    void rvb_reset();
    void doRun(double *, double *, long);	// was BASE::RVB()
    void matrix_mix();
    void uninit();
protected:
    int     rvbdelsize;
    double  Nsdelay[OutChannels][DelayCount];
    struct ReverbData {
        double delin;
        double Rand_info[6];
        double *Rvb_del;
        int deltap;
        double Rvb_air[3];
        double delout;
    } m_rvbData[OutChannels][DelayCount];
    // static methods
    static void get_primes(int x, int p[]);
    // static data
    static int	primes[NPRIMES + 2];
    static AtomicInt primes_gotten;
private:
    struct ReverbPatch {
        int incount;
        double *inptrs[MAX_INPUTS];
        double gains[MAX_INPUTS];
        double *outptr;
    } ReverbPatches[OutChannels*DelayCount];
    double		Allpass_del[OutChannels][502];
    int			allpassTap[OutChannels];
    double		m_rvbPast[OutChannels];	// For hi-pass filter
    double		AIRCOEFFS[NCOEFFS];
    double		Dimensions[5];
    double		m_dur, m_amp;
    float		*in;
    int			insamps;
    int			_branch;
};

#endif //_RVBBASE_H_
