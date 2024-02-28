/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Class for accessing the print buffer used for printing messages in
 'imbedded' RTcmix apps (rtcmix~, iRTcmix, etc.)
 The class is static for printer buffer data
 
*/

#ifndef __MM_PRINT_H__
#define __MM_PRINT_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	int get_mm_print_space();
	char *get_mm_print_ptr();
    char *get_mm_print_buf();
	void set_mm_print_ptr(int v);
	int is_print_cleared();
	void clear_print();
#ifdef __cplusplus
} // extern "C"
#endif

#endif	// __MM_PRINT_H__
