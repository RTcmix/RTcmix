//
// Created by Douglas Scott on 4/5/25.
//

#ifndef _QRVB_H_
#define _QRVB_H_

#include "RVBBASE.h"

#define QRVB_CHANS 4
#define QRVB_DELCOUNT 3

class QRVB : public RVBBASE<QRVB_CHANS, QRVB_DELCOUNT> {
public:
    QRVB();
    virtual ~QRVB();
protected:
    virtual void get_lengths(long);
    virtual void set_random();
};


#endif //_QRVB_H_
