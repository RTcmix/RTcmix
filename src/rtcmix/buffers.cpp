/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* Functions to handle allocation of audio buffers and copying between them.
                                                             -JGG, 17-Feb-00
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <globals.h>
#include <assert.h>

/* #define NDEBUG */     /* define to disable asserts */


/* local prototypes */
static BufPtr allocate_buffer(int nsamps);



/* -------------------------------------------------------- init_buf_ptrs --- */
/* Called from main. */
void
init_buf_ptrs()
{
   for (int i = 0; i < MAXBUS; i++) {
      audioin_buffer[i] = NULL;
      aux_buffer[i] = NULL;
      out_buffer[i] = NULL;
   }
}


/* ------------------------------------------------------ allocate_buffer --- */
static inline BufPtr
allocate_buffer(int nsamps)
{
   BufPtr buf_ptr;

   buf_ptr = (BUFTYPE *) calloc(nsamps, sizeof(BUFTYPE));

   return buf_ptr;
}


/* ---------------------------------------------- allocate_audioin_buffer --- */
// called from ??
int
allocate_audioin_buffer(short chan, int frames)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (audioin_buffer[chan] == NULL) {
      buf_ptr = allocate_buffer(frames);
      assert(buf_ptr != NULL);
      audioin_buffer[chan] = buf_ptr;
   }

   return 0;
}


/* -------------------------------------------------- allocate_aux_buffer --- */
// called from bus_config()
int
allocate_aux_buffer(short chan, int frames)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (aux_buffer[chan] == NULL) {
      buf_ptr = allocate_buffer(frames);
      assert(buf_ptr != NULL);
      aux_buffer[chan] = buf_ptr;
   }

   return 0;
}


/* -------------------------------------------------- allocate_out_buffer --- */
/* Called from rtsetparams. */
int
allocate_out_buffer(short chan, int frames)
{
   BufPtr buf_ptr;

   assert(chan >= 0 && chan < MAXBUS);

   if (out_buffer[chan] == NULL) {
      buf_ptr = allocate_buffer(frames);
      assert(buf_ptr != NULL);
      out_buffer[chan] = buf_ptr;
   }

   return 0;
}


