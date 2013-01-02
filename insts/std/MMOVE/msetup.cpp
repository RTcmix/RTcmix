// msetup.cpp -- setup routines for MPLACE and MMOVE
//

#include <stdlib.h>
#include <stdio.h>
#include <ugens.h>
#include "msetup.h"

static int    space_called = 0;
static int    matrix_flag = 0;
static int    _UseMikes = 0;
static float  _front, _right, _back, _left, _ceiling, _abs_factor, _rvb_time;
static double _MikeAngle = PI / 4.0;      /* in radians - default is 45 deg. */
static double _MikePatternFactor = 0.0;   /* 0 = omnidir, 1 = figure-8 */
static double _Matrix[12][12];
static double _Matrix_Gain = 0.72;
static AttenuationParams g_AttenParams = { 0.1, 300.0, 1.0 };

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

// These next routines are used to assure access to the RVB's input arrays

static int s_userCount = 0;

void
increment_users()
{
	++s_userCount;
}

void
decrement_users()
{
	--s_userCount;
}

int
check_users()
{
	return s_userCount;
}

/* ------------------------------------------------ get_rvb_setup_params --- */
/* Transfers Minc setup data to the RVB object, which continues some of the
   initialization formerly done in space().
   Returns 0 if successful, -1 if space hasn't been called in Minc score.
*/
int
get_rvb_setup_params(double Dimensions[],       /* array of 5 elements */
                 double Matrix[12][12],
                 float  *rvb_time)
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

   *rvb_time = _rvb_time;

   space_called = 1;

   return 0;
}

/* ----------------------------------------------------- get_setup_params --- */
/* Transfers Minc setup data to the PLACE object, which continues the
   initialization formerly done in space().
   Returns 0 if successful, -1 if space hasn't been called in Minc score.
*/
int
get_setup_params(double Dimensions[],       /* array of 5 elements */
				 AttenuationParams *params,
				 float *rvb_time,
                 float  *abs_factor,
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
   
   *params = g_AttenParams;

   *rvb_time = _rvb_time;
   *abs_factor = _abs_factor;
   *UseMikes = _UseMikes;
   *MikeAngle = _MikeAngle;
   *MikePatternFactor = _MikePatternFactor;

   space_called = 1;

   return 0;
}

/* ----------------------------------------------- set_attenuation_params --- */

double
m_set_attenuation_params(float p[], int n_args)
{
   if (n_args != 3) {
      die("set_attenuation_params",
	  	  "Usage: set_attenuation_params(min_dist, max_dist, dist_exponent)");
	  return -1;
   }
   if (p[0] < 0.1) {
      die("set_attenuation_params", "min distance must be > 0.1");
	  return -1;
   }
   if (p[1] < p[0] || p[1] > 300.0) {
      die("set_attenuation_params", "max distance must be >= min and < 300.0");
	  return -1;
   }
   if (p[2] < 0) {
      die("set_attenuation_params", "exponent must be >= 0");
	  return -1;
   }
   g_AttenParams.minDistance = p[0];
   g_AttenParams.maxDistance = p[1];
   g_AttenParams.distanceExponent = p[2];
   return 0;
}

/* ---------------------------------------------------------------- space --- */
/* This is the Minc setup routine for the binaural/room simulators, 
   MPLACE and MMOVE.
   It is invoked by the command space(), which takes arguments as follows:

      space (front, right, -back, -left, ceiling, abs_fac, rvbtime)

   The first four are the coordinates of the four walls, in relation to
   the listener, in feet. Since the listener is at point 0, the back and
   left walls must be specified as negative values. abs_fac is wall absorp-
   tion factor, between 0 (total absorption) and 10 (total reflection).   
*/
double
m_space(float p[], int n_args)
{
	if (space_called) {
      die("space", "'space' can only be called once");
	  return -1;
	}
   if (n_args < 7) {
      die("space",
	  	  "Usage: space(front, right, -back, -left, ceiling, absorb, rvbtime)");
	  return -1;
   }

   _front = p[0];
   _right = p[1];
   _back = p[2];
   _left = p[3];
   _ceiling = p[4];

   if (_back >= 0.0 || _left >= 0.0) {
      die("space", "'back' and 'left' wall coords must be negative");
	  return -1;
   }

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
m_mikes(float p[], int n_args)
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
m_mikes_off(float p[], int n_args)
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
m_oldmatrix(float p[], int n_args)
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
m_matrix(float p[], int n_args)
{
   int   i, j;
   float amp, val;

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
