/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// Classes for generating random numbers   JGG, 6/22/04
// Most of this code is based on C functions written by Luke Dubois.
// One class is based on code written by Mara Helmuth.

#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <RefCounted.h>

// Base class

class Random : public RefCounted {
public:
   Random(int aseed, double min, double max)
                  : _min(min), _max(max) { seed(aseed); }
   virtual double value() = 0;      // NB: not const, because it changes _randx
   void           seed(int aseed) { _randx = (long) aseed; }
protected:
   virtual        ~Random();
   double         rawvalue();
   double         fitrange(double num) const;
   double         getmax() const { return _max; }
   double         getmin() const { return _min; }
private:
   long           _randx;
   double         _min;
   double         _max;
};


// Subclasses, one for each distribution type

class LinearRandom : public Random {
public:
   LinearRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~LinearRandom();
};

class LowLinearRandom : public Random {
public:
   LowLinearRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~LowLinearRandom();
};

class HighLinearRandom : public Random {
public:
   HighLinearRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~HighLinearRandom();
};

class TriangleRandom : public Random {
public:
   TriangleRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~TriangleRandom();
};

class GaussianRandom : public Random {
public:
   GaussianRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~GaussianRandom();
};

class CauchyRandom : public Random {
public:
   CauchyRandom(int seed, double min = 0.0, double max = 1.0);
   virtual double value();
protected:
   virtual        ~CauchyRandom();
};

class MaraRandom : public Random {
public:
   MaraRandom(int seed, double min, double mid, double max, double tight);
   virtual double value();
protected:
   virtual        ~MaraRandom();
private:
   double         _mid;
   double         _tight;
};

#endif  // _RANDOM_H_

