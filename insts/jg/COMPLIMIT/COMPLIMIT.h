#include <objlib.h>

typedef enum {
   PEAK_DETECTOR = 0,
   RMS_DETECTOR,
   AVERAGE_DETECTOR
} DetectType;

typedef enum {
   ENV_INACTIVE,
   ENV_ATTACK,
   ENV_SUSTAIN,
   ENV_RELEASE
} EnvStage;

class COMPLIMIT : public Instrument {
   int         insamps, skip, inchan, atk_samps, rel_samps, bypass;
   int         window_len, env_count, branch, peak_under_thresh;
   float       ingain, outgain, threshold, ratio, pctleft;
   float       oamp, env_level, scale, atk_increment, rel_increment;
   float       target_scale, scale_increment, prev_peak, peak;
   float       *amptable, amptabs[2], *in;
   DetectType  detector_type;
   EnvStage    env_stage;
   RMS         *rms_gauge;

public:
   COMPLIMIT();
   virtual ~COMPLIMIT();
   int init(float *, short);
   int run();
private:
   float get_peak(int);
   float get_compress_factor(void);
};

