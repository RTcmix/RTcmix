#define MAXFILTER 64

#ifdef __cplusplus
extern "C" {
#endif

int get_iir_filter_specs(float cf[MAXFILTER], float bw[MAXFILTER],
                                                        float amp[MAXFILTER]);

#ifdef __cplusplus
} /* extern "C" */
#endif

