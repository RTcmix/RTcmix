#include <bus.h>        /* for MAXBUS */
#include <mixn_structs.h>

class MIXN : public Instrument {
	float amp,*amptable,tabs[2],*in;
	int skip;
	int amp_count;
	float inskip,outskip,dur,inchan;  // Put here so can be rtupdatable

	// Was in funcs.h
	loc aud_locs[MAXBUS];
	pt spk_locs[MAXBUS]; /* Do we have a variable for this? */
	int cur_point=0;
	rfact ratefs[MAXRATES];
	int max_rates;
	int cur_rate=0;
	int max_points;
	float out_chan_amp[MAXBUS]; /* Used by inst */
	double tot_dist;
	int n_spk;
	Bool use_path;
	Bool use_rates;
	double cycle;  /* Length of 1 iteration ... last path time */
	
public:
	MIXN();
	virtual ~MIXN();
	int init(float*, int);
	int run();
	};
