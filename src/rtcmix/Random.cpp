/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "Random.h"
#include <math.h>
#include <assert.h>

// Classes for generating random numbers   JGG, 6/22/04
// Most of this code is based on C functions written by Luke Dubois.
// One class is based on code written by Mara Helmuth.

inline double dmax(double x, double y) { return (x > y) ? x : y; }
inline double dmin(double x, double y) { return (x < y) ? x : y; }


// Base class -----------------------------------------------------------------

Random::~Random() {}

// Return a random number in range [0, 1]
inline double Random::rawvalue()
{
   _randx = (_randx * 1103515245) + 12345;
   int k = (_randx >> 16) & 077777;
   return (double) k / 32768.0;
}

// Scale <num>, which must be in range [0, 1], to fit range [_min, _max]
inline double Random::fitrange(double num) const
{
   assert(num >= 0.0 && num <= 1.0);
   return _min + (num * (_max - _min));
}


// Linear distribution subclass -----------------------------------------------

LinearRandom::LinearRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

LinearRandom::~LinearRandom() {}

double LinearRandom::value()
{
   return fitrange(rawvalue());
}


// Low-weighted linear distribution subclass ----------------------------------

LowLinearRandom::LowLinearRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

LowLinearRandom::~LowLinearRandom() {}

double LowLinearRandom::value()
{
   double num1 = rawvalue();
   double num2 = rawvalue();
   return fitrange(dmin(num1, num2));
}


// High-weighted linear distribution subclass ---------------------------------

HighLinearRandom::HighLinearRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

HighLinearRandom::~HighLinearRandom() {}

double HighLinearRandom::value()
{
   double num1 = rawvalue();
   double num2 = rawvalue();
   return fitrange(dmax(num1, num2));
}


// Triangle distribution subclass ---------------------------------------------

TriangleRandom::TriangleRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

TriangleRandom::~TriangleRandom() {}

double TriangleRandom::value()
{
   double num1 = rawvalue();
   double num2 = rawvalue();
   double tmp = 0.5 * (num1 + num2);
   return fitrange(tmp);
}


// Gaussian distribution subclass ---------------------------------------------

GaussianRandom::GaussianRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

GaussianRandom::~GaussianRandom() {}

double GaussianRandom::value()
{
   const int N = 12;
   const double halfN = 6.0;
   const double scale = 1.0;
   const double mu = 0.5;
   const double sigma = 0.166666;

   double num;
   do {
      num = 0.0;
      for (int j = 0; j < N; j++)
         num += rawvalue();
      num = sigma * scale * (num - halfN) + mu;
   } while (num < 0.0 || num > 1.0);

   return fitrange(num);
}


// Cauchy distribution subclass -----------------------------------------------

CauchyRandom::CauchyRandom(int seed, double min, double max)
   : Random(seed, min, max) {}

CauchyRandom::~CauchyRandom() {}

double CauchyRandom::value()
{
   const double alpha = 0.00628338;

   double num;
   do {
      do {
         num = rawvalue();
      } while (num == 0.5);
      num = (alpha * tan(num * M_PI)) + 0.5;
   } while (num < 0.0 || num > 1.0);

   return fitrange(num);
}


// Probability distribution subclass ------------------------------------------
// Based on code by Mara Helmuth

MaraRandom::MaraRandom(int seed, double min, double mid, double max,
                                                                double tight)
   : Random(seed, min, max), _mid(mid), _tight(tight) {}

MaraRandom::~MaraRandom() {}

double MaraRandom::value()
{
   double min = getmin();
   double max = getmax();
   double hirange = max - _mid;
   double lowrange = _mid - min;
   double range = dmax(hirange, lowrange);

   double num;
   do {
      double sign;
      num = rawvalue();       // num is [0,1]
      if (num > 0.5)
         sign = 1.0;
      else
         sign = -1.0;
      num = _mid + (sign * (pow(rawvalue(), _tight) * range));
   } while (num < min || num > max);

   return num;
}

