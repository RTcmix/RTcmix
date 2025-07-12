//
// Created by Douglas Scott on 4/3/25.
//

#include <ugens.h>
#include <rt.h>
#include <rtdefs.h>
#include "../MOVE/common.h"
#include "MOVEBASE.h"
#include <stdio.h>

template<int OutChannels>
MOVEBASE<OutChannels>::MOVEBASE(int bufsamps)
{
    R_old = -100000.0;
    T_old = 0.0;
    m_updateCount = 0;
    m_updateSamps = bufsamps;
    for (int n = 0; n < OutChannels; n++)
        for (int o = 0; o < 13; o++)
            oldOutlocs[n][o] = LocationUnset;	// to force update
}

template<int OutChannels>
MOVEBASE<OutChannels>::~MOVEBASE()
{
}

template<int OutChannels>
void MOVEBASE<OutChannels>::getVecTap(double *tapdel, int tapsize, double *Sig, double outloc, double *oldOutloc, int currentSamp, int len)
{
    double delta;
    if (*oldOutloc == LocationUnset)
        delta = 0.0;
    else
        delta = outloc - *oldOutloc;
    *oldOutloc = outloc;

    double incr = 1.0 + delta / len;

    const int tap = currentSamp % tapsize;
    double outTap = (double) tap - outloc;
    if (outTap < 0.0) outTap += tapsize;
    int out = 0;

    int len1 = (tapsize - (outTap + 1.0)) / incr;     // boundary point before needing to wrap
    int count = 0;
    int sampsNeeded = len;
    while (sampsNeeded > 0 && count++ < len1) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
//        if (tapdel[iOuttap] != 0.0) printf("getVecTap: tapdel[%d] = %f\n", iOuttap, tapdel[iOuttap]);
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[iOuttap+1] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
    while (sampsNeeded > 0 && outTap < (double) tapsize) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
        int outTapPlusOne = int(outTap + 1.0);
        if (outTapPlusOne >= tapsize)
            outTapPlusOne -= tapsize;
//        if (tapdel[iOuttap] != 0.0) printf("getVecTap: tapdel[%d] = %f\n", iOuttap, tapdel[iOuttap]);
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[outTapPlusOne] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
    outTap -= tapsize;
    while (sampsNeeded > 0) {
        const int iOuttap = (int) outTap;
        const double frac = outTap - (double)iOuttap;
//        if (tapdel[iOuttap] != 0.0) printf("getVecTap: tapdel[%d] = %f\n", iOuttap, tapdel[iOuttap]);
        Sig[out++] = tapdel[iOuttap] + frac * (tapdel[iOuttap+1] - tapdel[iOuttap]);
        outTap += incr;
        --sampsNeeded;
    }
}

template class MOVEBASE<2>;
template class MOVEBASE<4>;
