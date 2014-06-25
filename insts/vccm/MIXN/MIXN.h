#include "mixn_structs.h"

class MIXN : public Instrument {
	float amp, tabs[2],*in;
	double *amptable;
	int skip;
	int amp_count;
	float inskip,outskip,dur,inchan;  // Put here so can be rtupdatable
	// Was in funcs.h
	loc *my_aud_locs;
	pt *my_spk_locs; /* Do we have a variable for this? */
	rfact *my_ratefs;
	int my_num_rates;
	int my_cur_rate;
	int my_cur_point;
	int my_num_points;
	float *my_out_chan_amp; /* Used by inst */
	double my_tot_dist;
	int my_n_spk;
	Bool my_use_path;
	Bool my_use_rates;
	double my_cycle;  /* Length of 1 iteration ... last path time */

public:
	MIXN();
	virtual ~MIXN();
	int init(double*, int);
	int configure();
	int run();
};


