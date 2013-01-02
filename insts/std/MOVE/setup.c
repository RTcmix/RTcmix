#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include "setup.h"

static int    space_called = 0;
static int    matrix_flag = 0;
static int    _UseMikes = 0;
static float  _front, _right, _back, _left, _ceiling, _abs_factor, _rvb_time;
static double _MikeAngle = PI / 4.0;      /* in radians - default is 45 deg. */
static double _MikePatternFactor = 0.0;   /* 0 = omnidir, 1 = figure-8 */
static double _Matrix[12][12];
static double _Matrix_Gain = 0.72;


/* ---------------------------------------------------------- fill_matrix --- */
static void
fill_matrix()
{
   int    i, j;
   static double default_matrix[12][12] = {
      {  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0  },
      {  0,  0,  0,  0,  0, -1,  0,  0,  1,  0,  0,  0  },
      {  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0  },
      {  0,  0,  0,  0,  1,  0,  0, -1,  0,  0,  0,  0  },
      {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1,  0  },
      {  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  -1 },
      {  0,  0,  0,  0, -1,  0,  0,  0,  0,  1,  0,  0  },
      {  0,  0,  0,  1,  0,  0,  0,  0, -1,  0,  0,  0  },
      {  -1, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1  },
      {  0,  0,  0,  0,  0,  1,  0, -1,  0,  0,  0,  0  },
      {  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0  },
      {  0,  1,  0,  0,  0,  0,  0,  0,  0, -1,  0,  0  }
   };

   if (matrix_flag)
      return;                     /* matrix already loaded in score file */

   for (i = 0; i < 12; i++)
      for (j = 0; j < 12; j++)
         _Matrix[j][i] = _Matrix_Gain * default_matrix[j][i];
}


/* ----------------------------------------------------- get_setup_params --- */
/* Transfers Minc setup data to the PLACE object, which continues the
   initialization formerly done in space().
   Returns 0 if successful, -1 if space hasn't been called in Minc score.
*/
int
get_setup_params(double Dimensions[],       /* array of 5 elements */
                 double Matrix[12][12],
                 float  *abs_factor,
                 float  *rvb_time,
                 int    *UseMikes,
                 double *MikeAngle,
                 double *MikePatternFactor)
{
   int    i, j;

   if (!space_called)
      return -1;

   Dimensions[0] = (double)_front;
   Dimensions[1] = (double)_right;
   Dimensions[2] = (double)_back;
   Dimensions[3] = (double)_left;
   Dimensions[4] = (double)_ceiling;

   fill_matrix();

   /* copy local matrix into the one passed from caller */
   for (i = 0; i < 12; i++)
      for (j = 0; j < 12; j++)
         Matrix[j][i] = _Matrix[j][i];

   *abs_factor = _abs_factor;
   *rvb_time = _rvb_time;
   *UseMikes = _UseMikes;
   *MikeAngle = _MikeAngle;
   *MikePatternFactor = _MikePatternFactor;

   space_called = 1;

   return 0;
}


/* ---------------------------------------------------------------- space --- */
/* This is the Minc setup routine for the binaural/room simulator, PLACE.
   It is invoked by the command space(), which takes arguments as follows:

      space (front, right, -back, -left, ceiling, abs_fac, rvbtime)

   The first four are the coordinates of the four walls, in relation to
   the listener, in feet. Since the listener is at point 0, the back and
   left walls must be specified as negative values. abs_fac is wall absorp-
   tion factor, between 0 (total absorption) and 10 (total reflection).   
*/
double
space(float p[], int n_args)
{
   if (n_args < 7) {
      die(NULL, "Not enough args for `space'");
   }

   _front = p[0];
   _right = p[1];
   _back = p[2];
   _left = p[3];
   _ceiling = p[4];

   _abs_factor = p[5];

   _rvb_time = p[6];
   if (!_rvb_time)
      _rvb_time = 0.001;            /* shortest rvb time allowed */

   space_called = 1;

   return 0.0;
}


/* ---------------------------------------------------------------- mikes --- */
/* setup for microphone simulation
   syntax: mikes(mikeAngle, pattern)
*/
double
mikes(float p[], int n_args)
{
   _MikeAngle = p[0] * PI / 180.0;  /* convert to rads */
   _MikePatternFactor = (p[1] <= 1.0) ? p[1] : 1.0;
   rtcmix_advise("mikes", "Microphone angles: %.1f degrees, Pattern factor: %.1f",
          p[0], _MikePatternFactor);
   _UseMikes = 1;

   return 0.0;
}


/* ------------------------------------------------------------ mikes_off --- */
/* to turn off mike usage in order to use binaural filters
*/
double
mikes_off(float p[], int n_args)
{
   rtcmix_advise("mikes", "Microphone usage turned off.\n");
   _UseMikes = 0;

   return 0.0;
}


/* --------------------------------------------------------------- matrix --- */
/* This routine loads the 12 x 12 values from the Minc score file into the
   global array, Matrix, which is used by the RVB routine in PLACE. If it
   is not called, 'space' will fill array with default values.

      p0 = scales each element of the matrix

   if p0 is zero, resets to default matrix. In this case, the score MUST NOT
   contain matrix vals.
*/
double
oldmatrix(float p[], int n_args)
{
   int   i, j;
   float amp, val;

   amp = p[0];

   if (amp) {
      /* loop for 12 by 12 values on screen */
      for (i = 0; i < 12; i++) {
         for (j = 0; j < 12; j++) {
            scanf(" %f ", &val);
            _Matrix[i][j] = val * amp;
         }
      }
      rtcmix_advise("matrix", "Matrix loaded.\n");
      matrix_flag = 1;
   }
   else
      matrix_flag = 0;

   return 0.0;
}

double
matrix(float p[], int n_args)
{
   int   i, j;
   float amp;

   amp = p[0];

   if (amp) {
	  if (n_args == 1)
	  {
	   	_Matrix_Gain = amp;
		rtcmix_advise("matrix", "Default matrix.  Gain set to %g", amp);
		return 0;
	  }
   	  else if (n_args != 145)
	  {
	  	rtcmix_warn("matrix", "Incorrect number of args.  Ignoring matrix.");
		return 0;
	  }
      /* loop for 12 by 12 args */
      for (i = 0; i < 12; i++) {
         for (j = 0; j < 12; j++) {
            _Matrix[i][j] = p[12*i+j+1] * amp;
         }
      }
      rtcmix_advise("matrix", "Loaded 12x12 values.\n");
      matrix_flag = 1;
   }
   else
      matrix_flag = 0;

   return 0.0;
}
