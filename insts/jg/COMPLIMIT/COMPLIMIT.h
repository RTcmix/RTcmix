
typedef enum {
   PEAK_DETECTOR = 0,
   AVERAGE_DETECTOR = 1,
   RMS_DETECTOR = 2
} DetectType;

typedef enum {
   ENV_INACTIVE,
   ENV_ATTACK_WAIT,
   ENV_ATTACK,
   ENV_SUSTAIN,
   ENV_RELEASE
} EnvState;

class COMPLIMIT : public Instrument {
   bool        first_time, bypass;
   int         nargs, inchan, atk_samps, rel_samps, branch;
   int         lookahead_samps, offset, window_frames, window_len, buf_samps;
   int         wins_under_thresh, env_count, sus_count, next_env_count;
   float       inamp, outamp, gain, dbref, threshold_amp, pan;
   float       target_peak, target_gain, next_target_gain, gain_increment;
   float       *in, *inptr, *readptr, *bufstartptr;
   float       amptabs[2];
   double      *amptable;
   double      ingain, outgain, atk_time, rel_time, ratio, threshold_db;
   double      threshold_dbfs;
   DetectType  detector_type;
   EnvState    env_state;

   DetectType getDetectType(double pval);
   int usage();
   void doupdate();

public:
   COMPLIMIT();
   virtual ~COMPLIMIT();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
private:
   float get_peak(int, int *);
   float get_gain_reduction(void);
};

