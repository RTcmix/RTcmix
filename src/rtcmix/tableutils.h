/* This file contains prototypes for functions used by the RTcmix core,
   not by instruments or utility programs.   -JGG
*/
#ifndef _TABLEUTILS_H_ 
#define TABLEUTILS_H_ 1

#include <rtcmix_types.h>

void fill_linebrk_table(const Arg [], const int, double *, const int);
void fill_wave_table(const Arg [], const int, double *, const int);
int wavetable_from_string(const char *, double *, const int, const char *);

#endif /* _TABLEUTILS_H_ */
