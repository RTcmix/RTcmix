/* This file contains prototypes for functions used by the RTcmix core,
   not by instruments or utility programs.   -JGG
*/
#ifndef _TABLEUTILS_H_ 
#define TABLEUTILS_H_ 1

#include "rtcmix_types.h"

class PField;

bool is_table(const PField *pf);
void get_table_bounds(const double *array, int len, double &min, double &max);
double get_table_mean(const double *array, int len);
void fill_linebrk_table(const Arg [], int, double *, int);
void fill_wave_table(const Arg [], int, double *, int);
int wavetable_from_string(const char *, double *, int, const char *);

#endif /* _TABLEUTILS_H_ */
