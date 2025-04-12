//
// MOVEBASE: Common methods and state between DMOVE and QMOVE
// Created by Douglas Scott on 4/3/25.
//

#ifndef _MOVEBASE_H
#define _MOVEBASE_H

static const double LocationUnset = -999999.999999;

template<int OutChannels>
class MOVEBASE {
public:
    MOVEBASE(int bufsamps);
    virtual ~MOVEBASE();
protected:
    void getVecTap(double *tapdel, int tapsize, double *Sig, double outloc, double *oldOutloc, int currentSamp, int len);
protected:
    double R_old, T_old, mindiff;
    int m_updateSamps, m_updateCount;
    double oldOutlocs[OutChannels][13];
};

#endif //_MOVEBASE_H
