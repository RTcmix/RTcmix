#ifndef _MIXN_FUNCS_H_
#define _MIXN_FUNCS_H_

extern loc *aud_locs;
extern pt *spk_locs;
extern rfact *ratefs;
extern int num_rates;
extern int cur_rate;
extern int cur_point;
extern int num_points;
extern float *out_chan_amp; /* Used by inst */
extern double tot_dist;
extern int n_spk;
extern Bool use_path;
extern Bool use_rates;
extern double cycle;  /* Length of 1 iteration ... last path time */

#ifdef __cplusplus
extern "C" void update_amps(long);
#endif

#endif // _MIXN_FUNCS_H_
