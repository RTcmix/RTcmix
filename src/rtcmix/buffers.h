/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _BUFFERS_H_
#define _BUFFERS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* type of buffer used for internal buses */
#define BUFTYPE float           /* could be double some day */
typedef BUFTYPE *BufPtr;

/* type of buffer used to send to or receive from audio device */
#ifdef LINUX
#define OBUFTYPE short
typedef OBUFTYPE *OBufPtr;
#define LIMIT_OBUF_FOR_AUDIO_DEV
#define IBUFTYPE short
typedef IBUFTYPE *IBufPtr;
#endif /* LINUX */
#ifdef MACOSX
#define OBUFTYPE float        /* could make this float ... */
#define LIMIT_OBUF_FOR_AUDIO_DEV
typedef OBUFTYPE *OBufPtr;
#define IBUFTYPE float
typedef IBUFTYPE *IBufPtr;
#endif /* MACOSX */
#ifdef SGI
#define OBUFTYPE short        /* could make this float, as in CMC version ... */
#define LIMIT_OBUF_FOR_AUDIO_DEV  /* ...and then maybe comment this out */
typedef OBUFTYPE *OBufPtr;
#define IBUFTYPE short
typedef IBUFTYPE *IBufPtr;
#endif /* SGI */

void copy_interleaved_buf_to_one_buf(BufPtr dest, const BufPtr src,
                                int src_chans, int src_chan, int dest_frames);
void copy_one_buf_to_interleaved_buf(BufPtr dest, const BufPtr src,
                              int dest_chans, int dest_chan, int dest_frames);
void copy_interleaved_buf_to_buf(BufPtr dest, const BufPtr src, int dest_chans,
                 int dest_chan, int dest_frames, int src_chans, int src_chan);
void init_buf_ptrs(void);
void clear_audioin_buffers(void);
void clear_aux_buffers(void);
void clear_output_buffers(void);
int allocate_audioin_buffer(short chan, int nsamps);
int allocate_aux_buffer(short chan, int nsamps);
int allocate_out_buffer(short chan, int nsamps);
IBufPtr allocate_ibuf_ptr(int nsamps);
OBufPtr allocate_obuf_ptr(int nsamps);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _BUFFERS_H_ */
