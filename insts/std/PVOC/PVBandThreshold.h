//
//  PVBandThreshold.h
//  RTcmix
//
//  Created by Douglas Scott on 10/22/24.
//
//

#ifndef PVBandThreshold_h
#define PVBandThreshold_h

#include "PVFilter.h"

class PVBandThreshold : public PVFilter
{
public:
    static PVFilter *	create();
    virtual void        setFrameDuration(double duration);
    virtual int			run(float *pvdata, int nvals);
    virtual int			init(double *pp, int args);
protected:
    PVBandThreshold();
    virtual 			~PVBandThreshold();
private:
    float               _frameDuration;
    float               _attackTime;
    float               _decayTime;
    float               _ampThreshold;
    float               _attackIncrement;
    float               _decayIncrement;
    float *				_binGains;
};

#endif //PVBandThreshold_h
