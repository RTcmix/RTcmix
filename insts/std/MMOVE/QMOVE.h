//
// QMOVE.h -- 4-channel version of MMOVE */
// Created by Douglas Scott on 3/30/25.
//

#ifndef _QMOVE_H_
#define _QMOVE_H_

#include "MBASE.h"
#include "MOVEBASE.h"

class QMOVE : public MBASE, public MOVEBASE<4> {
public:
    QMOVE();
    virtual ~QMOVE();
    virtual int configure();
    virtual int run();
protected:
    virtual int checkOutputChannelCount();
    virtual int localInit(double *p, int n_args);
    virtual int alloc_vectors();
    virtual int finishInit(double *ringdur);
    virtual int updatePosition(int currentSamp);
    virtual void get_tap(int currentSamp, int chan, int path, int len);
};

#endif //_QMOVE_H_
