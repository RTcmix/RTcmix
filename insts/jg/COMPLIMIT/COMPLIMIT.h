
typedef enum {
   PEAK_DETECTOR = 0,
   AVERAGE_DETECTOR,
   RMS_DETECTOR
} DetectType;

typedef enum {
   ENV_INACTIVE,
   ENV_ATTACK_WAIT,
   ENV_ATTACK,
   ENV_SUSTAIN,
   ENV_RELEASE
} EnvState;

class COMPLIMIT : public Instrument {
   int         skip, inchan, atk_samps, rel_samps, bypass, branch, first_time;
   int         lookahead_samps, offset, window_frames, window_len, buf_samps;
   int         wins_under_thresh, env_count, sus_count, next_env_count;
   float       ingain, outgain, gain, threshold, pctleft, oamp, dbref;
   float       target_peak, target_gain, next_target_gain, gain_increment;
   float       *in, *inptr, *readptr, *bufstartptr;
   float       amptabs[2];
   double      *amptable;
   double      ratio;
   DetectType  detector_type;
   EnvState    env_state;

public:
   COMPLIMIT();
   virtual ~COMPLIMIT();
   int init(double p[], int n_args);
   int run();
private:
   float get_peak(int, int *);
   float get_gain_reduction(void);
};

