/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#ifndef _BUFFERS_H_
#define _BUFFERS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BUFTYPE float             /* could be double some day */

typedef (BUFTYPE *) BufPtr;

void init_buf_ptrs(void);
int allocate_audioin_buffer(short chan);
int allocate_aux_buffer(short chan);
int allocate_out_buffer(short chan);
void copy_interleaved_buf_to_one_buf(BufPtr dest, const BufPtr src,
                                int src_chans, int src_chan, int dest_frames);
void copy_one_buf_to_interleaved_buf(BufPtr dest, const BufPtr src,
                              int dest_chans, int dest_chan, int dest_frames);
void copy_interleaved_buf_to_buf(BufPtr dest, const BufPtr src, int dest_chans,
                 int dest_chan, int dest_frames, int src_chans, int src_chan);



/* inlined functions -------------------------------------------------------- */

/* -------------------------------------- copy_interleaved_buf_to_one_buf --- */
/* Copy the specified channel of an interleaved buffer to a one-channel
   buffer. Buffers must be of same type (e.g., float).
*/
inline void
copy_interleaved_buf_to_one_buf(
      BufPtr         dest,            /* buffer containing one chan */
      const BufPtr   src,             /* interleaved buffer */
      int            src_chans,       /* chans in interleaved buffer */
      int            src_chan,        /* channel to copy from */
      int            dest_frames)     /* frames in destination buffer */
{
   int   i, j;

   for (i = 0, j = src_chan; i < dest_frames; i++, j += src_chans)
      dest[i] = src[j];
}


/* -------------------------------------- copy_one_buf_to_interleaved_buf --- */
/* Copy a one-channel buffer into the specified channel of an interleaved
   buffer. Buffers must be of same type (e.g., float).
*/
inline void
copy_one_buf_to_interleaved_buf(
      BufPtr         dest,            /* interleaved buffer */
      const BufPtr   src,             /* buffer containing one chan */
      int            dest_chans,      /* chans in interleaved buffer */
      int            dest_chan,       /* channel to copy into */
      int            dest_frames)     /* frames in destination buffer */
{
   int   i, j;

   for (i = 0, j = dest_chan; i < dest_frames; i++, j += dest_chans)
      dest[j] = src[i];
}


/* ------------------------------------------ copy_interleaved_buf_to_buf --- */
/* Copy one channel between two interleaved buffers. Buffers must be
   of same type (e.g., float).
*/
inline void
copy_interleaved_buf_to_buf(
      BufPtr         dest,            /* interleaved buffer */
      const BufPtr   src,             /* interleaved buffer */
      int            dest_chans,      /* number of chans interleaved */
      int            dest_chan,       /* chan to copy into */
      int            dest_frames,     /* frames in dest buffer */
      int            src_chans,       /* number of chans interleaved */
      int            src_chan)        /* chan to copy from */
{
   int   i, j, k;

   j = src_chan;
   k = dest_chan;
   for (i = 0; i < dest_frames; i++) {
      dest[k] = src[j];
      src_chan += src_chans;
      dest_chan += dest_chans;
   }
}


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _BUFFERS_H_ */
