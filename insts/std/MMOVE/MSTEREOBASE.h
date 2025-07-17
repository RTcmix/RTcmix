//
// Created by Douglas Scott on 3/30/25.
//

#ifndef _MSTEREOBASE_H_
#define _MSTEREOBASE_H_

#include "MBASE.h"

class MSTEREOBASE : public MBASE {
public:
    MSTEREOBASE();
    virtual ~MSTEREOBASE();
    virtual int configure();
    virtual int run();
protected:
    virtual int checkOutputChannelCount();
    virtual int alloc_vectors();
    virtual int alloc_firfilters();
    void earfil_set(int);
    struct BinauralVector : public MBASE::Vector {
        BinauralVector() : Firtaps(NULL), Fircoeffs(NULL) {}
        ~BinauralVector() { delete [] Fircoeffs; delete [] Firtaps; }
        void allocate(int length);
        virtual void runFilters(int currentSamp, int len, int pathIndex);
        double *Firtaps;
        double *Fircoeffs;
    };
};


#endif //_MSTEREOBASE_H_
